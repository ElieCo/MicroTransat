#include <Arduino_JSON.h>

struct Waypoint{
  Coord coord;
  double corridor_width;
  double valid_dist;
};

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
    db_dist_to_axis.init(m_db, "Dist_to_axis", float(0), true);
    db_in_corridor.init(m_db, "In_corridor", true, true);
    db_corridor_angle.init(m_db, "Corridor_angle", float(0));

    bool *sd_ready = m_db->initData("SD_ready", false);
    *sd_ready = m_mission_file.init("mission.txt", *sd_ready);
    parseMission();
  }

  void go(){
    // Calcul the distance to the next waypoint.
    float distanceToWaypoint = get_distance(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).coord.lat, m_waypoints.at(db_wpt_index.get()).coord.lng);

    // Check if the actual waypoint is validated and select the next one if needed.
    if (next_point(distanceToWaypoint)) {
      // Calcul the distance to the new waypoint.
      distanceToWaypoint = get_distance(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).coord.lat, m_waypoints.at(db_wpt_index.get()).coord.lng);
    }
    // Calcul the course to the next waypoint.
    float angleToWaypoint = get_course(db_latitude.get(), db_longitude.get(), m_waypoints.at(db_wpt_index.get()).coord.lat, m_waypoints.at(db_wpt_index.get()).coord.lng);

    // Set all this data in the DB.
    db_dist_to_wpt.set(distanceToWaypoint);
    db_angle_to_wpt.set(angleToWaypoint);
    db_lat_next.set(m_waypoints.at(db_wpt_index.get()).coord.lat);
    db_lng_next.set(m_waypoints.at(db_wpt_index.get()).coord.lng);
    db_lat_prev.set(m_waypoints.at(db_wpt_index.get() - 1).coord.lat);
    db_lng_prev.set(m_waypoints.at(db_wpt_index.get() - 1).coord.lng);
    db_corridor_width.set(m_waypoints.at(db_wpt_index.get()).corridor_width);

    // Check if we are in the corridor
    isInCorridor();
  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Default_corridor_width", m_default_corridor_width);
    m_db->getData("Default_validation_distance", m_default_validation_distance);
  }

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
  DBData<float> db_dist_to_axis;
  DBData<bool> db_in_corridor;
  DBData<float> db_corridor_angle;

  Vector<Waypoint> m_waypoints;
  double m_default_validation_distance;
  double m_default_corridor_width;

  SDfile m_mission_file;

  void parseMission(){
    String text = m_mission_file.readAll();

    JSONVar mission = JSON.parse(text);

    for (int i = 0; i < mission.length(); i++){

      if(mission[i].hasOwnProperty("latitude") && mission[i].hasOwnProperty("longitude")){
        Waypoint wp;
        wp.coord.lat = mission[i]["latitude"];
        wp.coord.lng = mission[i]["longitude"];

        if (mission[i].hasOwnProperty("corridor_width")) wp.corridor_width = mission[i]["corridor_width"];
        else wp.corridor_width = m_default_corridor_width;

        if (mission[i].hasOwnProperty("validation_distance")) wp.valid_dist = mission[i]["validation_distance"];
        else wp.valid_dist = m_default_validation_distance;

        m_waypoints.push_back(wp);
      }
    }


    db_lat_next.set(m_waypoints.at(db_wpt_index.get()).coord.lat);
    db_lng_next.set(m_waypoints.at(db_wpt_index.get()).coord.lng);
    db_lat_prev.set(m_waypoints.at(db_wpt_index.get() - 1).coord.lat);
    db_lng_prev.set(m_waypoints.at(db_wpt_index.get() - 1).coord.lng);
    db_corridor_width.set(m_waypoints.at(db_wpt_index.get()).corridor_width);
  }

  boolean next_point(float dist) { // unité : mètres
    // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
    // Note : we consider that if dist==0 there should be an error.
    if (dist <= m_waypoints.at(db_wpt_index.get()).valid_dist && dist != 0) {
      // Change the index.
      db_wpt_index.set((db_wpt_index.get()+1) % m_waypoints.size());
      return true;
    }
    return false;
  }

  void isInCorridor(){
    float angle_btw_wpt = get_course(db_lat_prev.get(), db_lng_prev.get(), db_lat_next.get(), db_lng_next.get());
    float dist_to_axis = sin(radians(angle_btw_wpt - db_angle_to_wpt.get())) * db_dist_to_wpt.get();

    bool in_corridor = abs(dist_to_axis) <= float(db_corridor_width.get())/2;

    db_corridor_angle.set(angle_btw_wpt);
    db_dist_to_axis.set(dist_to_axis);
    db_in_corridor.set(in_corridor);
  }

};
