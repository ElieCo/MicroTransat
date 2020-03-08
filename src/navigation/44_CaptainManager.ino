
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3 };

class Captain : public BaseManager
{
  void init(){
    m_behaviour = ACQUISITION;
    m_max_upwind = 30;
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
    float course;
    m_db->getData("Course", course);

    float angleToWaypoint;
    m_db->getData("Wpt_angle", angleToWaypoint);

    float reg_angle;
    m_db->getData("Regulator_angle", reg_angle);

    

    float diff = angleToWaypoint - course;
    from180to180(diff);

    float new_reg = reg_angle + diff;

    int sign = new_reg/new_reg;

    if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
    if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

    m_db->setData("Regulator_angle", new_reg);

    m_behaviour = PROCESS;
  }

  void state_process(){
    m_behaviour = SLEEP;
  }

  BEHAVIOUR m_behaviour;

  float m_max_upwind, m_max_downwind;
};

