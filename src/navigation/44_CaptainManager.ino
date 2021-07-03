
class Captain : public BaseManager
{
  public:
    BASIC_CONSTRUCTOR(Captain)
    ~Captain() {}

    void init() {
      m_behaviour = DataCaptainManager_Behaviour_ACQUISITION;
    }

    void go() {
      GetCaptainData.behaviour = m_behaviour;

      // Urgent check : if we are radio controlled
      if (GetSensorData.radio.radio_controlled)
        m_behaviour = DataCaptainManager_Behaviour_RADIO_CONTROLLED;

      switch (m_behaviour) {
        case DataCaptainManager_Behaviour_SLEEP:
          stateSleep();
          break;
        case DataCaptainManager_Behaviour_ACQUISITION:
          stateAcquisition();
          break;
        case DataCaptainManager_Behaviour_DECIDE:
          stateDecide();
          break;
        case DataCaptainManager_Behaviour_PROCESS:
          stateProcess();
          break;
        case DataCaptainManager_Behaviour_RADIO_CONTROLLED:
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
    }

    DataCaptainManager_Behaviour m_behaviour;

    double m_max_upwind, m_max_downwind, m_sleeping_duration;
    float m_prev_average_course;

    void stateSleep() {
      // Sleep

      static int timer = -1;
      if (timer == -1) timer = millis();
      else {
        if (millis() - timer > m_sleeping_duration) {
          timer = -1;

          // Say that we just wake up.
          GetCaptainData.just_wake_up = true;
          GetSensorData.gps.average_course_full = false;

          m_behaviour = DataCaptainManager_Behaviour_ACQUISITION;
        }
      }

    }

    void stateAcquisition() {
      // Wait that the missionManager update the setpoint
      if (!GetMissionData.setpoint_update_asked){
        // Wait that the course average buffer is full to take a decision.
        if (GetSensorData.gps.average_course_full) {
          GetCaptainData.just_wake_up = false;
          m_behaviour = DataCaptainManager_Behaviour_DECIDE;
        }
      }

    }

    void stateDecide() {

      if (GetMissionData.next_element.type == DataMissionManager_MissionElement_ElementType_WPT)
        commandForWPT();
      else
        commandForAWA();

      // make sure to wait that the helm manager process
      GetHelmData.cmd_applied = false;

      // Change the behaviour.
      m_behaviour = DataCaptainManager_Behaviour_PROCESS;
    }

    void stateProcess() {
      if (GetHelmData.cmd_applied)
        m_behaviour = DataCaptainManager_Behaviour_SLEEP;
    }

    void stateRadioControlled() {
      if (!GetSensorData.radio.radio_controlled){
        m_behaviour = DataCaptainManager_Behaviour_ACQUISITION;
        // Ask to update the setpoint.
        GetMissionData.setpoint_update_asked = true;
      }
    }

    void commandForWPT() {
      // Calculate the angle to the next waypoint
      float angleToWaypoint = get_course(GetSensorData.gps.coord.latitude, GetSensorData.gps.coord.longitude, GetMissionData.next_element.coord.latitude, GetMissionData.next_element.coord.longitude);

      // Calcul the difference between the actual course an the angle to the next waypoint.
      float diff = angleToWaypoint - GetSensorData.gps.average_course;

      // Calcul the new regulator command to reach the waypoint.
      float new_reg = GetCaptainData.helm_order + diff;
      from180to180(new_reg);

      float diff_wpt_corridor = angleToWaypoint - GetMissionData.corridor_angle;
      from180to180(diff_wpt_corridor);

      // If she's on the corridor, she go on the same direction as the previous regul, else reach the corridor (depend of if we go downwind or upwind).
      bool isPositive = true;
      if (GetMissionData.in_corridor) {
        isPositive = GetCaptainData.helm_order >= 0;
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

      // Set in the DB the regulator angle.
      GetCaptainData.helm_order = new_reg;
    }

    void commandForAWA() {
      // Get awa cmd
      float new_reg = GetMissionData.next_element.angle;
      from180to180(new_reg);

      // Check that it doesn't go out of range
      int sign = new_reg >= 0 ? 1 : -1;

      // Avoid to go less than *m_max_upwind* deg or more than *m_max_downwind*.
      if (abs(new_reg) < m_max_upwind) new_reg = sign * m_max_upwind;
      if (abs(new_reg) > m_max_downwind) new_reg = sign * m_max_downwind;

      // Set in the DB the regulator angle.
      GetCaptainData.helm_order = new_reg;
    }
};
