
enum BEHAVIOUR { SLEEP = 0, ACQUISITION = 1, DECIDE = 2, PROCESS = 3, RADIO_CONTROLLED = 4 };

class Captain : public BaseManager
{
  public:
    BASIC_CONSTRUCTOR(Captain)
    ~Captain() {}

    void init() {
      m_behaviour = ACQUISITION;

      db_reg_cmd.init(m_db, "Regulator_angle", float(0), true);
      db_in_corridor.init(m_db, "In_corridor", true, true);
      db_cmd_helm_applied.init(m_db, "Cmd_helm_applied", true);
      db_course.init(m_db, "Average_course", float(0));
      MissionElement* empty_elem = NULL;
      db_next_element.init(m_db, "Next_element", empty_elem);
      db_just_wake_up.init(m_db, "Just_wake_up", false);
      db_average_course_full.init(m_db, "Average_course_full", false);
      db_corridor_angle.init(m_db, "Corridor_angle", float(0));
      db_behaviour.init(m_db, "Behaviour", SLEEP, true);
      db_latitude.init(m_db, "Latitude", double(0));
      db_longitude.init(m_db, "Longitude", double(0));
      db_radio_controlled.init(m_db, "Radio_controlled", false);
      db_ask_setpoint_update.init(m_db, "Ask_setpoint_update", true);
      db_ask_add_awa_angle.init(m_db, "Ask_add_awa_angle", double(0));
    }

    void go() {
      db_behaviour.set(m_behaviour);

      // Urgent check : if we are radio controlled
      if (db_radio_controlled.get())
        m_behaviour = RADIO_CONTROLLED;

      switch (m_behaviour) {
        case SLEEP:
          stateSleep();
          break;
        case ACQUISITION:
          stateAcquisition();
          break;
        case DECIDE:
          stateDecide();
          break;
        case PROCESS:
          stateProcess();
          break;
        case RADIO_CONTROLLED:
          stateRadioControlled();
          break;
      }
    }

    void stop() {}

  private:

    void config() {
      m_db->getData("Max_upwind", m_max_upwind);
      m_db->getData("Max_downwind", m_max_downwind);
      m_db->getData("Sleeping_duration", m_sleeping_duration);
      m_db->getData("Too_far_for_tack", m_too_far_for_tack);
    }

    DBData<float> db_reg_cmd;
    DBData<bool> db_in_corridor;
    DBData<bool> db_cmd_helm_applied;
    DBData<float> db_course;
    DBData<MissionElement*> db_next_element;
    DBData<bool> db_just_wake_up;
    DBData<bool> db_average_course_full;
    DBData<float> db_corridor_angle;
    DBData<int> db_behaviour;
    DBData<double> db_latitude;
    DBData<double> db_longitude;
    DBData<bool> db_radio_controlled;
    DBData<bool> db_ask_setpoint_update;
    DBData<double> db_ask_add_awa_angle; // 0 don't ask, else ask this angle

    BEHAVIOUR m_behaviour;

    double m_max_upwind, m_max_downwind, m_sleeping_duration;
    double m_too_far_for_tack;
    float m_prev_average_course;

    void stateSleep() {
      // Sleep

      static int timer = -1;
      if (timer == -1) timer = millis();
      else {
        if (millis() - timer > m_sleeping_duration) {
          timer = -1;

          // Say that we just wake up.
          db_just_wake_up.set(true);
          db_average_course_full.set(false);

          m_behaviour = ACQUISITION;
        }
      }

    }

    void stateAcquisition() {
      // Wait that the missionManager update the setpoint
      if (!db_ask_setpoint_update.get()){
        // Wait that the course average buffer is full to take a decision.
        if (db_average_course_full.get()) {
          db_just_wake_up.set(false);
          m_behaviour = DECIDE;
        }
      }

    }

    void stateDecide() {

      // Decide of the command
      double cmd = 0;
      if (db_next_element.get()->type == WPT)
        cmd = commandForWPT();
      else
        cmd = commandForAWA();

      if (checkViolentTack(cmd)){
        // Force to update the setpoint

        // Change the behaviour.
        m_behaviour = ACQUISITION;
        // Ask to update the setpoint.
        db_ask_setpoint_update.set(true);

      } else {
        // Set in the DB the regulator angle command.
        db_reg_cmd.set(cmd);

        // Make sure to wait that the helm manager process
        db_cmd_helm_applied.set(false);

        // Change the behaviour.
        m_behaviour = PROCESS;
      }

    }

    void stateProcess() {
      if (db_cmd_helm_applied.get())
        m_behaviour = SLEEP;
    }

    void stateRadioControlled() {
      if (!db_radio_controlled.get()){
        m_behaviour = ACQUISITION;
        // Ask to update the setpoint.
        db_ask_setpoint_update.set(true);
      }
    }

    double commandForWPT() {
      // Get next wpt
      MissionElement wpt = *db_next_element.get();

      // Calculate the angle to the next waypoint
      float angleToWaypoint = get_course(db_latitude.get(), db_longitude.get(), wpt.coord.lat, wpt.coord.lng);

      // Calcul the difference between the actual course an the angle to the next waypoint.
      float diff = angleToWaypoint - db_course.get();

      // Calcul the new regulator command to reach the waypoint.
      float new_reg = db_reg_cmd.get() + diff;
      from180to180(new_reg);

      float diff_wpt_corridor = angleToWaypoint - db_corridor_angle.get();
      from180to180(diff_wpt_corridor);

      // If she's on the corridor, she go on the same direction as the previous regul, else reach the corridor (depend of if we go downwind or upwind).
      bool isPositive = true;
      if (db_in_corridor.get()) {
        isPositive = db_reg_cmd.get() >= 0;
      } else {
        if (abs(new_reg) < 90) {
          isPositive = diff_wpt_corridor >= 0;
        } else {
          isPositive = diff_wpt_corridor <= 0;
        }
      }
      int sign = isPositive ? 1 : -1;

      // Avoid to go less than *m_max_upwind* deg or more than *m_max_downwind*.
      if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
      if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

      return new_reg;
    }

    double commandForAWA() {
      // Get next awa
      MissionElement awa = *db_next_element.get();

      // Get awa cmd
      float new_reg = awa.angle;
      from180to180(new_reg);

      // Get the sign of the cmd
      int sign = new_reg >= 0 ? 1 : -1;

      // Avoid to go less than *m_max_upwind* deg or more than *m_max_downwind*.
      if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
      if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

      return new_reg;
    }

    bool checkViolentTack(double new_cmd){
      // Check if we try to do a tack while we are far away from the wind

      // Get actual command
      double actual_cmd = db_reg_cmd.get();

      // Check if we are far away from the wind and if we have to do a tack.
      bool is_far_away = abs(actual_cmd) > m_too_far_for_tack;
      bool have_to_tack = actual_cmd / new_cmd < 0; //if they don't have the same sign

      if (is_far_away && have_to_tack){
        int sign = actual_cmd < 0 ? -1 : 1;
        db_ask_add_awa_angle.set(sign * m_max_upwind);
        return true;
      }

      return false;
    }

};
