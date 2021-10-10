
#undef GetData
#undef GetConf
#define GetData m_main_data->captain_manager
#define GetConf m_main_conf->captain_manager

class Captain : public BaseManager
{
  public:
    BASIC_CONSTRUCTOR(Captain)
    ~Captain() {}

    void init() {
      m_behaviour = DataCaptainManager_Behaviour_ACQUISITION;
      m_sleeping_duration = GetConf.sleeping_duration
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

    DataCaptainManager_Behaviour m_behaviour;

    float m_sleeping_duration;

    void stateSleep() {
      // Sleep

      static int timer = -1;
      if (timer == -1) timer = millis();
      else {
        if (millis() - timer > m_sleeping_duration) {
          timer = -1;

          // Reset the sleeping duration to default
          m_sleeping_duration = GetConf.sleeping_duration;

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

      // Decide of the command
      double cmd = 0;
      if (GetMissionData.next_element.type == DataMissionManager_MissionElement_ElementType_WPT)
        cmd = commandForWPT();
      else
        cmd = commandForAWA();

      if (checkViolentTack(cmd)){
        // Force to update the setpoint

        // Change the behaviour.
        m_behaviour = DataCaptainManager_Behaviour_ACQUISITION;
        // Ask to update the setpoint.
        GetData.setpoint_update_asked = true;

      } else {
        // Set the regulator angle command.
        GetData.helm_order = cmd;

        // Make sure to wait that the helm manager process
        GetHelmData.cmd_applied = false;

        // Change the behaviour.
        m_behaviour = DataCaptainManager_Behaviour_PROCESS;
      }
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

    double commandForWPT() {
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

      // Avoid to go less than *GetConf.max_upwind* deg or more than *GetConf.max_downwind*.
      if (abs(new_reg) < GetConf.max_upwind) new_reg = sign * GetConf.max_upwind;
      if (abs(new_reg) > GetConf.max_downwind) new_reg = sign * GetConf.max_downwind;


      //print("Captain:", GetConf.max_downwind, GetConf.max_upwind);


      return new_reg;
    }

    double commandForAWA() {
      // Set the sleeping duration to the AWA duration
      m_sleeping_duration = 1000 * GetMissionData.next_element.duration;

      // Get awa cmd
      float new_reg = GetMissionData.next_element.angle;
      from180to180(new_reg);

      // Get the sign of the cmd
      int sign = new_reg >= 0 ? 1 : -1;

      // Avoid to go less than *GetConf.max_upwind* deg or more than *GetConf.max_downwind*.
      if (abs(new_reg) < GetConf.max_upwind) new_reg = sign * GetConf.max_upwind;
      if (abs(new_reg) > GetConf.max_downwind) new_reg = sign * GetConf.max_downwind;

      return new_reg;
    }

    bool checkViolentTack(double new_cmd){
      // Check if we try to do a tack while we are far away from the wind

      // Get actual command
      double actual_cmd = GetData.helm_order;

      // Check if we are far away from the wind and if we have to do a tack.
      bool is_far_away = abs(actual_cmd) > GetConf.too_far_for_tack;
      bool have_to_tack = (actual_cmd / new_cmd) < 0; //if they don't have the same sign

      if (is_far_away && have_to_tack){
        int sign = actual_cmd < 0 ? -1 : 1;
        GetData.ask_add_awa_angle = sign * GetConf.max_upwind;
        return true;
      }

      return false;
    }

};
