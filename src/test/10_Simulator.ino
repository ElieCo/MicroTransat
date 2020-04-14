

class Simulator{
  public:
  Simulator()
    : m_movement_period(100)
    , m_movement_time(0)
    , m_gps_period(500)
    , m_gps_time(0)
    , m_wind_direction(0.0)
    , m_actual_cap(0.0)
    , m_actual_speed(0.0)
    , m_average_course(10)
  {
    m_actual_position.lat = 0.0;
    m_actual_position.lng = 0.0;
  }
  ~Simulator(){}

  void initSimulation(DataBase* db, float wind_dir, Coord init_pos){
    m_db = db;
    m_wind_direction = wind_dir;
    m_actual_position = init_pos;
  }

  void simulateMovement(){

    if (m_movement_time == 0 || (millis() - m_movement_time) >= m_movement_period)
      m_movement_time = millis();
    else
      return;

    // Calcul new position with previous speed, cap and position
    calculatePosition(m_movement_period);

    // Get the new cap
    m_db->getData("Cmd_helm", m_actual_cap);
    from0to360(m_actual_cap);
    
    // Calcul new speed
    float wing_angle = 0.0;
    m_db->getData("Wing_angle", wing_angle);
    calculateSpeed(wing_angle);
    
  }

  void simulateGps(){
    
    if (m_gps_time == 0 || (millis() - m_gps_time) >= m_gps_period)
      m_gps_time = millis();
    else
      return;

    m_db->setData("Latitude", long(m_actual_position.lat*1000000));
    m_db->setData("Longitude", long(m_actual_position.lng*1000000));
    m_db->setData("Fix_age", int(0));

    int time = millis()/1000;
    int second = time % 60;
    int min = ((time - second)/60) % 60;
    int hour = (time - second - min*60) / 3600;
    time = String(String(hour)+(min<10 ? "0" : "")+String(min)+(second<10 ? "0" : "")+String(second)).toInt();
    
    m_db->setData("Time", time);
    m_db->setData("Date", int(0));
    m_db->setData("Speed", toKnots(m_actual_speed));
    m_db->setData("Course", m_actual_cap);
    m_db->setData("Average_course", m_average_course.average(m_actual_cap));
    m_db->setData("HDOP", int(42));
    m_db->setData("Gps_ready", true);
  }

  float getWindDirection(){
    return m_wind_direction;
  }
  void setWindDirection(float wind_dir){
    m_wind_direction = wind_dir;
    from0to360(m_wind_direction);
  }
  void moveWindDirection(float diff){
    m_wind_direction = m_wind_direction + diff;
    from0to360(m_wind_direction);
  }

  void setMovementPeriod(int period){
    m_movement_period = period;
  }
  void setGpsPeriod(int period){
    m_gps_period = period;
  }

  private:
  DataBase* m_db;

  unsigned int m_movement_period;
  unsigned int m_movement_time;
  unsigned int m_gps_period;
  unsigned int m_gps_time;

  float m_wind_direction;
  float m_actual_cap;
  float m_actual_speed;
  Coord m_actual_position;

  Average<float> m_average_course;

  void calculateSpeed(float wing_angle){

    float lift_angle = 0.0;
    if (wing_angle < 0) lift_angle = m_wind_direction + (wing_angle + 90);
    else lift_angle = m_wind_direction - (wing_angle + 90);
    from0to360(lift_angle);

    float diff_angle = abs(m_actual_cap - lift_angle);
    float prop_coeff = cos(radians(diff_angle));
    
    float max_speed = 1.5;
    float theorical_speed = max_speed * prop_coeff;

    m_actual_speed = m_actual_speed + 0.3*(theorical_speed - m_actual_speed);
  }

  void calculatePosition(float dt){
    float dist = m_actual_speed * dt/1000;
    double nlat, nlng;
    getPointAtDistAndBearing(m_actual_position.lat, m_actual_position.lng, dist, m_actual_cap, nlat, nlng);
    m_actual_position.lat = nlat;
    m_actual_position.lng = nlng;
  }
};

