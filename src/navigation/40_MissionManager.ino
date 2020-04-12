#include <Arduino_JSON.h>


class MissionManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(MissionManager)
  ~MissionManager(){}

  void init(){
    m_db->initData("Wpt_dist", float());
    m_db->initData("Wpt_angle", float());
    m_db->initData("Lat_next_point", int());
    m_db->initData("Lon_next_point", int());
    m_db->initData("Wpt_index", int());

    m_mission_file.init("mission.txt");
    parseMission();
  }

  void go(){
    // Get actual coordintes.
    long lat, lng;
    m_db->getData("Latitude", lat);
    m_db->getData("Longitude", lng);

    // Calcul the distance to the next waypoint.
    float distanceToWaypoint = get_distance(float(lat) / 1000000, float(lng) / 1000000, m_waypoints.at(m_index).lat, m_waypoints.at(m_index).lng);

    // Check if the actual waypoint is validated and select the next one if needed.
    if (next_point(distanceToWaypoint)) {
      // Calcul the distance to the new waypoint.
      distanceToWaypoint = get_distance(float(lat) / 1000000, float(lng) / 1000000, m_waypoints.at(m_index).lat, m_waypoints.at(m_index).lng);
    }
    // Calcul the course to the next waypoint.
    float angleToWaypoint = get_course(float(lat) / 1000000, float(lng) / 1000000, m_waypoints.at(m_index).lat, m_waypoints.at(m_index).lng);

    // Set all this data in the DB.
    m_db->setData("Wpt_dist", distanceToWaypoint);
    m_db->setData("Wpt_angle", angleToWaypoint);
    m_db->setData("Lat_next_point", int(m_waypoints.at(m_index).lat * 1000000));
    m_db->setData("Lon_next_point", int(m_waypoints.at(m_index).lng * 1000000));
  }

  void stop(){}

  private:

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
  }

  boolean next_point(float dist) { // unité : mètres
    // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
    // Note : we consider that if dist==0 there should be an error.
    if (dist <= m_valid_wpt && dist != 0) {
      // Change the index.
      if (m_index < m_waypoints.size() - 1) {
        m_index ++;
      }
      else {
        m_index = 0;
      }
      // Set in the DB the new index.
      m_db->setData("Wpt_index", m_index);
      return true;
    }
    return false;
  }


  //float wp_lat[2] = {47.731309, 47.730627};
  //float wp_lon[2] = { -3.395384, -3.390921};
  Vector<Coord> m_waypoints;
  int m_index = 0;
  int m_valid_wpt = 25;

  SDfile m_mission_file;

};
