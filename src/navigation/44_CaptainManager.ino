
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3 };

class Captain : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(Captain)
  ~Captain(){}

  void init(){
    m_behaviour = ACQUISITION;
    m_db->initData("Regulator_angle", float(), true);
    m_db->initData("Dist_to_axis", float(), true);
    m_db->initData("In_corridor", bool(), true);
  }

  void go(){
    switch(m_behaviour){
      case SLEEP:
        state_sleep();
        break;
      case ACQUISITION:
        state_acquisition();
        break;
      case DECIDE:
        state_decide();
        break;
      case PROCESS:
        state_process();
        break;
    }
  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Max_upwind", m_max_upwind);
    m_db->getData("Max_downwind", m_max_downwind);
  }

  BEHAVIOUR m_behaviour;

  double m_max_upwind, m_max_downwind;

  void state_sleep(){
    m_behaviour = ACQUISITION;
  }

  void state_acquisition(){
    m_behaviour = DECIDE;
  }

  void state_decide(){
    // Get the actual course.
    float course;
    m_db->getData("Average_course", course);

    // Get the angle to the next waypoint.
    float angleToWaypoint;
    m_db->getData("Wpt_angle", angleToWaypoint);

    // Get the angle of the regulator.
    float reg_angle;
    m_db->getData("Cmd_helm", reg_angle);
    from180to180(reg_angle);

    // Calcul the difference between the actual course an the angle to the next waypoint.
    float diff = angleToWaypoint - course;

    // Calcul the new regulator command to reach the waypoint.
    float new_reg = reg_angle + diff;
    from180to180(new_reg);

    // if she's on the corridor, she go on the same direction as the previous regul, else as the new one
    bool isPositive = isInCorridor() ? reg_angle >= 0 : new_reg >= 0;
    int sign = isPositive ? 1 : -1;

    // Avoid to go less than *m_max_upwind* deg or more than *m_max_downwind*.
    if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
    if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

    // Set in the DB the regulator angle.
    m_db->setData("Regulator_angle", new_reg);

    // Change the behaviour.
    m_behaviour = PROCESS;
  }

  void state_process(){
    m_behaviour = SLEEP;
  }

  bool isInCorridor(){
    
    float angleToWaypoint = 0;
    m_db->getData("Wpt_angle", angleToWaypoint);
    
    float distToWaypoint = 0;
    m_db->getData("Wpt_dist", distToWaypoint);
    
    int lat_next_point = 0;
    m_db->getData("Lat_next_point", lat_next_point);
    
    int lon_next_point = 0;
    m_db->getData("Lon_next_point", lon_next_point);
    
    int lat_prev_point = 0;
    m_db->getData("Lat_prev_point", lat_prev_point);
    
    int lon_prev_point = 0;
    m_db->getData("Lon_prev_point", lon_prev_point);
    
    int corridor_width = 0;
    m_db->getData("Corridor_width", corridor_width);

    float angle_btw_wpt = get_course(float(lat_prev_point)/1000000, float(lon_prev_point)/1000000, float(lat_next_point)/1000000, float(lon_next_point)/1000000);
    float dist_to_axis = sin(radians(angle_btw_wpt - angleToWaypoint)) * distToWaypoint;
    
    bool in_corridor = abs(dist_to_axis) <= corridor_width/2;
    
    m_db->setData("Dist_to_axis", dist_to_axis);
    m_db->setData("In_corridor", in_corridor);

    return in_corridor;
    
  }
};
