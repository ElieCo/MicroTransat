
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3 };

class Captain : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(Captain)
  ~Captain(){}

  void init(){
    m_behaviour = ACQUISITION;
    m_max_upwind = 30;
    m_db->setData("Max_upwind", m_max_upwind);
    m_max_downwind = 130;
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

  private:

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
    m_db->getData("Regulator_angle", reg_angle);


    // Calcul the difference between the actual course an the angle to the next waypoint.
    float diff = angleToWaypoint - course;
    from180to180(diff);

    // Calcul the new regulator command to reach the waypoint.
    float new_reg = reg_angle + diff;

    int sign = new_reg/new_reg;

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

  BEHAVIOUR m_behaviour;

  float m_max_upwind, m_max_downwind;
};
