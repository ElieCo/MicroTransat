


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
  }

  void go(){
    // Get actual coordintes.
    long lat, lng;
    m_db->getData("Latitude", lat);
    m_db->getData("Longitude", lng);

    // Calcul the distance to the next waypoint.
    float distanceToWaypoint = get_distance(float(lat) / 1000000, float(lng) / 1000000, wp_lat[m_index], wp_lon[m_index]);

    // Check if the actual waypoint is validated and select the next one if needed.
    if (next_point(distanceToWaypoint)) {
      // Calcul the distance to the new waypoint.
      distanceToWaypoint = get_distance(float(lat) / 1000000, float(lng) / 1000000, wp_lat[m_index], wp_lon[m_index]);
    }
    // Calcul the course to the next waypoint.
    float angleToWaypoint = get_course(float(lat) / 1000000, float(lng) / 1000000, wp_lat[m_index], wp_lon[m_index]);

    // Set all this data in the DB.
    m_db->setData("Wpt_dist", distanceToWaypoint);
    m_db->setData("Wpt_angle", angleToWaypoint);
    m_db->setData("Lat_next_point", int(wp_lat[m_index] * 1000000));
    m_db->setData("Lon_next_point", int(wp_lon[m_index] * 1000000));
  }

  void stop(){}

  private:

  boolean next_point(float dist) { // unité : mètres
    // If the distance between the boat and the waypoint is less than *m_valid_wpt* m.
    // Note : we consider that if dist==0 there should be an error.
    if (dist <= m_valid_wpt && dist != 0) {
      // Change the index.
      if (m_index < (sizeof(wp_lat) / sizeof(float)) - 1) {
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

  float wp_lat[2] = {47.731309, 47.730627};
  float wp_lon[2] = { -3.395384, -3.390921};
  unsigned m_index = 0;
  int m_valid_wpt = 25;
};
