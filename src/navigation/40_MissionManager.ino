#include <Arduino_JSON.h>

#define MISSION_FILENAME "mission.txt"

enum MissionElementType { WPT = 0, AWA = 1 };

class MissionElement : public ObjectForDB {
  public:
    MissionElement(bool ephemeral = false)
      : ObjectForDB()
      , ephemeral(ephemeral)
    {}

    Coord coord;
    double corridor_width = -1;
    double valid_dist = -1;

    double angle = 90;
    double duration = 30;

    String out = "MissionElement";

    MissionElementType type = WPT;

    bool ephemeral;

    String toString() {
      String result = "";
      if (type == WPT) {
        result += "WPT/";
        result += String(coord.lat, 7) + "/";
        result += String(coord.lng, 7) + "/";
        result += String(corridor_width) + "/";
        result += String(valid_dist);
      } else {
        result += "AWA/";
        result += String(angle, 7) + "/";
        result += String(duration, 7) + "/";
      }
      return result;
    }
};

class MissionManager : public BaseManager
{
  public:
    BASIC_CONSTRUCTOR(MissionManager)
    ~MissionManager() {}

    void init() {
      m_awa_start_time = 0;

      GetLogData.sd_ready = m_mission_file.init(MISSION_FILENAME, GetLogData.sd_ready);
      parseMission();
    }

    void go() {
      // If we are radio controlled reset the index to play the first element
      if (GetSensorData.radio.radio_controlled) {
        // Insert an ephemeral element
        if (!m_mission_elements.at(GetMissionData.element_index).ephemeral) {
          DataMissionManager_MissionElement awa;
          awa.ephemeral = true;
          awa.angle = m_auto_start_angle;
          awa.duration = m_auto_start_duration;
          awa.type = DataMissionManager_MissionElement_ElementType_AWA;
          m_mission_elements.insert(GetMissionData.element_index, awa);

          // Reload the element at GetMissionData.element_index place.
          GetMissionData.element_index--;
          runNextElement();
        }

      } else {
        // Update setpoints
        updateSetpoints();

        // Check if the actual element is validated and select the next one if needed.
        if (checkActualElementFinished()) runNextElement();

        // Say that the setpoint is updated
        GetMissionData.setpoint_update_asked = false;

      }

    }

    void stop() {}

  private:

    void config() {
      m_db->getData("Default_corridor_width", m_default_corridor_width);
      m_db->getData("Default_validation_distance", m_default_validation_distance);
      m_db->getData("Auto_start_angle", m_auto_start_angle);
      m_db->getData("Auto_start_duration", m_auto_start_duration);
    }

    Vector<DataMissionManager_MissionElement> m_mission_elements;
    double m_default_validation_distance, m_default_corridor_width;
    double m_auto_start_angle, m_auto_start_duration;
    unsigned long m_awa_start_time;

    SDfile m_mission_file;

    void parseMission() {

      // Parse the mission file
      String text = m_mission_file.readAll();

      JSONVar mission = JSON.parse(text);

      for (int i = 0; i < mission.length(); i++) {

        if (mission[i].hasOwnProperty("latitude") && mission[i].hasOwnProperty("longitude")) {
          DataMissionManager_MissionElement wp;
          wp.coord.latitude = mission[i]["latitude"];
          wp.coord.longitude = mission[i]["longitude"];

          if (mission[i].hasOwnProperty("corridor_width")) wp.corridor_width = mission[i]["corridor_width"];
          else wp.corridor_width = m_default_corridor_width;

          if (mission[i].hasOwnProperty("validation_distance")) wp.valid_dist = mission[i]["validation_distance"];
          else wp.valid_dist = m_default_validation_distance;

          wp.type = DataMissionManager_MissionElement_ElementType_WPT;

          m_mission_elements.push_back(wp);
        }
        else if (mission[i].hasOwnProperty("angle") && mission[i].hasOwnProperty("duration")) {
          DataMissionManager_MissionElement awa;
          awa.angle = mission[i]["angle"];
          awa.duration = mission[i]["duration"];

          awa.type = DataMissionManager_MissionElement_ElementType_AWA;

          m_mission_elements.push_back(awa);
        }
      }

      GetMissionData.element_index = 0;
      GetMissionData.next_element = m_mission_elements.at(GetMissionData.element_index);
      GetMissionData.prev_element = m_mission_elements.at(GetMissionData.element_index - 1);
    }

    bool checkActualElementFinished() {
      DataMissionManager_MissionElement elem = m_mission_elements.at(GetMissionData.element_index);

      if (elem.type == DataMissionManager_MissionElement_ElementType_WPT) {
        // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
        // Note : we consider that if dist==0 there should be an error.
        if (GetMissionData.wpt_dist <= elem.valid_dist && GetMissionData.wpt_dist != 0) return true;
        else return false;

      } else {
        // Check awa duration since the first time we check it (for radio purpose)
        if (m_awa_start_time == 0) m_awa_start_time = millis();

        // Get the duration since the awa element is started;
        unsigned long awa_duration = millis() - m_awa_start_time;

        if (awa_duration >= elem.duration * 1000) return true;
        else return false;
      }
    }

    void runNextElement() {
      // If the element was an ephemeral element, delete it and change the index
      if (m_mission_elements.at(GetMissionData.element_index).ephemeral) {
        m_mission_elements.removeAt(GetMissionData.element_index);
        GetMissionData.element_index--;
        GetMissionData.next_element = m_mission_elements.at(GetMissionData.element_index);
      }

      // Change the index.
      int next_index = (GetMissionData.element_index + 1) % m_mission_elements.size();
      GetMissionData.element_index = next_index;

      // Get the new actual element
      GetMissionData.next_element = m_mission_elements.at(GetMissionData.element_index);

      // Get the prev actual element
      GetMissionData.prev_element = m_mission_elements.at(GetMissionData.element_index - 1);

      // Reset start time if it's an AWA element
      if (GetMissionData.next_element.type == DataMissionManager_MissionElement_ElementType_AWA) {
        m_awa_start_time = 0;
      }

      updateSetpoints();
    }

    void updateSetpoints() {
      if (GetMissionData.next_element.type == DataMissionManager_MissionElement_ElementType_WPT) {
        // Get new distance to waypoint
        float distanceToWaypoint = get_distance(GetSensorData.gps.coord.latitude, GetSensorData.gps.coord.longitude, GetMissionData.next_element.coord.latitude, GetMissionData.next_element.coord.longitude);
        // Calcul the course to the next waypoint.
        float angleToWaypoint = get_course(GetSensorData.gps.coord.latitude, GetSensorData.gps.coord.longitude, GetMissionData.next_element.coord.latitude, GetMissionData.next_element.coord.longitude);
        // Set all this data in the DB.
        GetMissionData.wpt_dist = distanceToWaypoint;
        GetMissionData.wpt_angle = angleToWaypoint;

        // Check if we are in the corridor
        isInCorridor();
      }
    }

    void isInCorridor() {
      DataMissionManager_MissionElement prev = GetMissionData.prev_element;
      DataMissionManager_MissionElement next = GetMissionData.next_element;
      float angle_btw_wpt = get_course(prev.coord.latitude, prev.coord.longitude, next.coord.latitude, next.coord.longitude);
      float dist_to_axis = sin(radians(angle_btw_wpt - GetMissionData.wpt_angle)) * GetMissionData.wpt_dist;

      bool in_corridor = abs(dist_to_axis) <= float(next.corridor_width) / 2;

      GetMissionData.corridor_angle = angle_btw_wpt;
      GetMissionData.dist_to_axis = dist_to_axis;
      GetMissionData.in_corridor = in_corridor;
    }

};
