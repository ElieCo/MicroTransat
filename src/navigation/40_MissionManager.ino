


class MissionManager : public BaseManager
{
  public:
  ~MissionManager(){}

  void go(){
    long lat, lng;
    m_db->getData("Latitude", lat);
    m_db->getData("Longitude", lng);

    float distanceToWaypoint = get_distance(float(lat) / 1000000, float(lon) / 1000000, wp_lat[m_index], wp_lon[m_index]);
    if (next_point(distanceToWaypoint)) {
      distanceToWaypoint = get_distance(float(lat) / 1000000, float(lon) / 1000000, wp_lat[m_index], wp_lon[m_index]);
    }
    float angleToWaypoint = get_course(float(lat) / 1000000, float(lon) / 1000000, wp_lat[m_index], wp_lon[m_index]);

    m_db->setData("Wpt_dist", distanceToWaypoint);
    m_db->setData("Wpt_angle", angleToWaypoint);
    m_db->setData("Lat_next_point", int(wp_lat[m_index] * 1000000));
    m_db->setData("Lon_next_point", int(wp_lon[m_index] * 1000000));
  }

  private:

  boolean next_point(float dist) { // unité : mètres
    if (dist <= m_valid_wpt && dist != 0) { // && GPS.satellites > 3 // dist = 0 -> sacrément improbable !
      if (m_index < (sizeof(wp_lat) / sizeof(float)) - 1) {
        m_index ++;
      }
      else {
        m_index = 0;
      }
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

