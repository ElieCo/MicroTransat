#include <Arduino_JSON.h>


class MissionManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(MissionManager)
  ~MissionManager(){}

  void init(){
    db_dist_to_wpt.init(m_db, "Wpt_dist", float(0), true);
    db_angle_to_wpt.init(m_db, "Wpt_angle", float(0), true);
    db_lat_next.init(m_db, "Lat_next_point", double(0), true);
    db_lng_next.init(m_db, "Lon_next_point", double(0), true);
    db_lat_prev.init(m_db, "Lat_prev_point", double(0), true);
    db_lng_prev.init(m_db, "Lon_prev_point", double(0), true);
    db_corridor_width.init(m_db, "Corridor_width", int(100), true);
    db_wpt_index.init(m_db, "Wpt_index", int(0), true);
    db_latitude.init(m_db, "Latitude", double(0));
    db_longitude.init(m_db, "Longitude", double(0));

    bool *sd_ready = m_db->initData("SD_ready", false);
    *sd_ready = m_mission_file.init("mission.txt", *sd_ready);
    parseMission();
  }

  void go(){
    // Calcul the distance to the next waypoint.
    float distanceToWaypoint = get_distance(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).lat, m_waypoints.at(db_wpt_index.get()).lng);

    // Check if the actual waypoint is validated and select the next one if needed.
    if (next_point(distanceToWaypoint)) {
      // Calcul the distance to the new waypoint.
      distanceToWaypoint = get_distance(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).lat, m_waypoints.at(db_wpt_index.get()).lng);
    }
    // Calcul the course to the next waypoint.
    float angleToWaypoint = get_course(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).lat, m_waypoints.at(db_wpt_index.get()).lng);

    // Set all this data in the DB.
    db_dist_to_wpt.set(distanceToWaypoint);
    db_angle_to_wpt.set(angleToWaypoint);
    db_lat_next.set(m_waypoints.at(db_wpt_index.get()).lat);
    db_lng_next.set(m_waypoints.at(db_wpt_index.get()).lng);
    db_lat_prev.set(m_waypoints.at(db_wpt_index.get() - 1).lat);
    db_lng_prev.set(m_waypoints.at(db_wpt_index.get() - 1).lng);
  }

  void stop(){}

  private:

  DBData<float> db_dist_to_wpt;
  DBData<float> db_angle_to_wpt;
  DBData<double> db_lat_next;
  DBData<double> db_lng_next;
  DBData<double> db_lat_prev;
  DBData<double> db_lng_prev;
  DBData<int> db_corridor_width;
  DBData<int> db_wpt_index;
  DBData<double> db_latitude;
  DBData<double> db_longitude;

  Vector<Coord> m_waypoints;
  int m_valid_wpt = 25;
  int m_corridor_width = 60;

  SDfile m_mission_file;

  void parseMission(){
    String text = m_mission_file.readAll();

    JSONVar mission = JSON.parse(text);

    if (mission.hasOwnProperty("waypoints")) {
      JSONVar waypoints = mission["waypoints"];
      m_waypoints.clear();
      for (int i = 0; i < waypoints.length(); i++){
        Coord wp;
        wp.lat = double(waypoints[i][0]);
        wp.lng = double(waypoints[i][1]);
        m_waypoints.push_back(wp);
      }
    }

    db_lat_next.set(m_waypoints.at(db_wpt_index.get()).lat);
    db_lng_next.set(m_waypoints.at(db_wpt_index.get()).lng);
    db_lat_prev.set(m_waypoints.at(db_wpt_index.get() - 1).lat);
    db_lng_prev.set(m_waypoints.at(db_wpt_index.get() - 1).lng);
    db_corridor_width.set(m_corridor_width);
  }

  boolean next_point(float dist) { // unité : mètres
    // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
    // Note : we consider that if dist==0 there should be an error.
    if (dist <= m_valid_wpt && dist != 0) {
      // Change the index.
      db_wpt_index.set((db_wpt_index.get()+1) % m_waypoints.size());
      return true;
    }
    return false;
  }

};
