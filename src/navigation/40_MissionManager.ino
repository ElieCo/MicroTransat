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

      db_dist_to_wpt.init(m_db, "Wpt_dist", float(0), true);
      db_angle_to_wpt.init(m_db, "Wpt_angle", float(0), true);
      db_elem_index.init(m_db, "Elem_index", int(0), true);
      db_latitude.init(m_db, "Latitude", double(0));
      db_longitude.init(m_db, "Longitude", double(0));
      db_dist_to_axis.init(m_db, "Dist_to_axis", float(0), true);
      db_in_corridor.init(m_db, "In_corridor", true, true);
      db_corridor_angle.init(m_db, "Corridor_angle", float(0));
      MissionElement* empty_elem = NULL;
      db_elem_prev.init(m_db, "Prev_element", empty_elem, true);
      db_elem_next.init(m_db, "Next_element", empty_elem, true);

      db_radio_controlled.init(m_db, "Radio_controlled", true);

      db_ask_setpoint_update.init(m_db, "Ask_setpoint_update", true);

      bool *sd_ready = m_db->initData("SD_ready", false);
      *sd_ready = m_mission_file.init(MISSION_FILENAME, *sd_ready);
      parseMission();
    }

    void go() {
      // If we are radio controlled reset the index to play the first element
      if (db_radio_controlled.get()) {
        // Insert an ephemeral element
        if (!m_mission_elements.at(db_elem_index.get()).ephemeral) {
          MissionElement awa = MissionElement(true);
          awa.angle = m_auto_start_angle;
          awa.duration = m_auto_start_duration;
          awa.type = AWA;
          m_mission_elements.insert(db_elem_index.get(), awa);

          // Reload the element at db_elem_index.get() place.
          db_elem_index.add(-1);
          runNextElement();
        }

      } else {
        // Update setpoints
        updateSetpoints();

        // Check if the actual element is validated and select the next one if needed.
        if (checkActualElementFinished()) runNextElement();

        // Say that the setpoint is updated
        db_ask_setpoint_update.set(false);

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

    DBData<float> db_dist_to_wpt;
    DBData<float> db_angle_to_wpt;
    DBData<MissionElement*> db_elem_prev;
    DBData<MissionElement*> db_elem_next;
    DBData<int> db_elem_index;
    DBData<double> db_latitude;
    DBData<double> db_longitude;
    DBData<float> db_dist_to_axis;
    DBData<bool> db_in_corridor;
    DBData<float> db_corridor_angle;

    DBData<bool> db_radio_controlled;

    DBData<bool> db_ask_setpoint_update;

    Vector<MissionElement> m_mission_elements;
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
          MissionElement wp;
          wp.coord.lat = mission[i]["latitude"];
          wp.coord.lng = mission[i]["longitude"];

          if (mission[i].hasOwnProperty("corridor_width")) wp.corridor_width = mission[i]["corridor_width"];
          else wp.corridor_width = m_default_corridor_width;

          if (mission[i].hasOwnProperty("validation_distance")) wp.valid_dist = mission[i]["validation_distance"];
          else wp.valid_dist = m_default_validation_distance;

          wp.type = WPT;

          m_mission_elements.push_back(wp);
        }
        else if (mission[i].hasOwnProperty("angle") && mission[i].hasOwnProperty("duration")) {
          MissionElement awa;
          awa.angle = mission[i]["angle"];
          awa.duration = mission[i]["duration"];

          awa.type = AWA;

          m_mission_elements.push_back(awa);
        }
      }

      db_elem_index.set(0);
      db_elem_next.set(m_mission_elements.ptrAt(db_elem_index.get()));
      db_elem_prev.set(m_mission_elements.ptrAt(db_elem_index.get() - 1));
    }

    bool checkActualElementFinished() {
      MissionElement elem = m_mission_elements.at(db_elem_index.get());

      if (elem.type == WPT) {
        // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
        // Note : we consider that if dist==0 there should be an error.
        if (db_dist_to_wpt.get() <= elem.valid_dist && db_dist_to_wpt.get() != 0) return true;
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
      if (m_mission_elements.at(db_elem_index.get()).ephemeral) {
        m_mission_elements.removeAt(db_elem_index.get());
        db_elem_index.add(-1);
        db_elem_next.set(m_mission_elements.ptrAt(db_elem_index.get()));
      }

      // Change the index.
      int next_index = (db_elem_index.get() + 1) % m_mission_elements.size();
      db_elem_index.set(next_index);

      // Get the new actual element
      MissionElement *new_elem = m_mission_elements.ptrAt(db_elem_index.get());

      // Get the prev actual element
      MissionElement *prev_elem = m_mission_elements.ptrAt(db_elem_index.get() - 1);

      db_elem_prev.set(prev_elem);
      db_elem_next.set(new_elem);

      // Reset start time if it's an AWA element
      if (db_elem_next.get()->type == AWA) {
        m_awa_start_time = 0;
      }

      updateSetpoints();
    }

    void updateSetpoints() {
      if (db_elem_next.get()->type == WPT) {
        // Get new distance to waypoint
        float distanceToWaypoint = get_distance(db_latitude.get(), db_longitude.get(), db_elem_next.get()->coord.lat, db_elem_next.get()->coord.lng);
        // Calcul the course to the next waypoint.
        float angleToWaypoint = get_course(db_latitude.get(), db_longitude.get(), db_elem_next.get()->coord.lat, db_elem_next.get()->coord.lng);
        // Set all this data in the DB.
        db_dist_to_wpt.set(distanceToWaypoint);
        db_angle_to_wpt.set(angleToWaypoint);

        // Check if we are in the corridor
        isInCorridor();
      }
    }

    void isInCorridor() {
      MissionElement *prev = static_cast<MissionElement*>(db_elem_prev.get());
      MissionElement *next = static_cast<MissionElement*>(db_elem_next.get());
      float angle_btw_wpt = get_course(prev->coord.lat, prev->coord.lng, next->coord.lat, next->coord.lng);
      float dist_to_axis = sin(radians(angle_btw_wpt - db_angle_to_wpt.get())) * db_dist_to_wpt.get();

      bool in_corridor = abs(dist_to_axis) <= float(next->corridor_width) / 2;

      db_corridor_angle.set(angle_btw_wpt);
      db_dist_to_axis.set(dist_to_axis);
      db_in_corridor.set(in_corridor);
    }

};
