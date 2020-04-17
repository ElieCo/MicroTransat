
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3 };

class Captain : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(Captain)
  ~Captain(){}

  void init(){
    m_behaviour = ACQUISITION;
    m_db->initData("Regulator_angle", float(0), true);
    m_db->initData("Dist_to_axis", float(0), true);
    m_db->initData("In_corridor", true, true);
    m_db->initData("Cmd_helm_applied", true);
    
    m_prev_average_course = 0;
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
  float m_prev_average_course;

  void state_sleep(){
    m_behaviour = ACQUISITION;
  }

  void state_acquisition(){
    float course = 0;
    m_db->getData("Average_course", course);

    // Wait that the course average is stable to take a decision.
    if(m_prev_average_course != 0 && abs(m_prev_average_course - course) < 5) {
      m_behaviour = DECIDE;
      m_prev_average_course = 0;
    } else {
      m_prev_average_course = course;
    }
    
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

    // make sure to wait that the helm manager process
    m_db->setData("Cmd_helm_applied", false);

    // Change the behaviour.
    m_behaviour = PROCESS;
  }

  void state_process(){
    bool helm_applied = true;
    m_db->getData("Cmd_helm_applied", helm_applied);
    if (helm_applied)
      m_behaviour = SLEEP;
  }

  bool isInCorridor(){
    
    float angleToWaypoint = 0;
    m_db->getData("Wpt_angle", angleToWaypoint);
    
    float distToWaypoint = 0;
    m_db->getData("Wpt_dist", distToWaypoint);
    
    double lat_next_point = 0;
    m_db->getData("Lat_next_point", lat_next_point);
    
    double lon_next_point = 0;
    m_db->getData("Lon_next_point", lon_next_point);
    
    double lat_prev_point = 0;
    m_db->getData("Lat_prev_point", lat_prev_point);
    
    double lon_prev_point = 0;
    m_db->getData("Lon_prev_point", lon_prev_point);
    
    int corridor_width = 0;
    m_db->getData("Corridor_width", corridor_width);

    float angle_btw_wpt = get_course(lat_prev_point, lon_prev_point, lat_next_point, lon_next_point);
    float dist_to_axis = sin(radians(angle_btw_wpt - angleToWaypoint)) * distToWaypoint;
    
    bool in_corridor = abs(dist_to_axis) <= corridor_width/2;
    
    m_db->setData("Dist_to_axis", dist_to_axis);
    m_db->setData("In_corridor", in_corridor);

    return in_corridor;
    
  }
};
