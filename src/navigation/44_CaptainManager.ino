
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3 };

class Captain : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(Captain)
  ~Captain(){}

  void init(){
    m_behaviour = ACQUISITION;

    db_reg_cmd.init(m_db, "Regulator_angle", float(0), true);
    db_in_corridor.init(m_db, "In_corridor", true, true);
    db_cmd_helm_applied.init(m_db, "Cmd_helm_applied", true);
    db_course.init(m_db, "Average_course", float(0));
    db_wpt_angle.init(m_db, "Wpt_angle", float(0));
    db_just_wake_up.init(m_db, "Just_wake_up", false);
    db_average_course_full.init(m_db, "Average_course_full", false);
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

  DBData<float> db_reg_cmd;
  DBData<bool> db_in_corridor;
  DBData<bool> db_cmd_helm_applied;
  DBData<float> db_course;
  DBData<float> db_wpt_angle;
  DBData<bool> db_just_wake_up;
  DBData<bool> db_average_course_full;

  BEHAVIOUR m_behaviour;

  double m_max_upwind, m_max_downwind;
  float m_prev_average_course;

  void state_sleep(){
    // Sleep

    // Say that we just wake up.
    db_just_wake_up.set(true);
    db_average_course_full.set(false);

    m_behaviour = ACQUISITION;
  }

  void state_acquisition(){
    // Wait that the course average buffer is full to take a decision.
    if(db_average_course_full.get()) {
      db_just_wake_up.set(false);
      m_behaviour = DECIDE;
    }

  }

  void state_decide(){
    // Calcul the difference between the actual course an the angle to the next waypoint.
    float diff = db_wpt_angle.get() - db_course.get();

    // Calcul the new regulator command to reach the waypoint.
    float new_reg = db_reg_cmd.get() + diff;
    from180to180(new_reg);

    // if she's on the corridor, she go on the same direction as the previous regul, else as the new one
    bool isPositive = db_in_corridor.get() ? db_reg_cmd.get() >= 0 : new_reg >= 0;
    int sign = isPositive ? 1 : -1;

    // Avoid to go less than *m_max_upwind* deg or more than *m_max_downwind*.
    if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
    if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

    // Set in the DB the regulator angle.
    db_reg_cmd.set(new_reg);

    // make sure to wait that the helm manager process
    db_cmd_helm_applied.set(false);

    // Change the behaviour.
    m_behaviour = PROCESS;
  }

  void state_process(){
    if (db_cmd_helm_applied.get())
      m_behaviour = SLEEP;
  }
};
