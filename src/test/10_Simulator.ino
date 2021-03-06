

class Simulator{
  public:
  Simulator()
    : m_movement_period(100)
    , m_movement_time(0)
    , m_gps_period(500)
    , m_gps_time(0)
    , m_wind_origin(0.0)
    , m_actual_cap(0.0)
    , m_actual_speed(0.0)
    , m_course_average(3)
  {
    m_actual_position.lat = 0.0;
    m_actual_position.lng = 0.0;
  }
  ~Simulator(){}

  void initSimulation(DataBase* db, float wind_origin, Coord init_pos){
    m_db = db;
    m_wind_origin = wind_origin;
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
    float helm = 0.0;
    m_db->getData("Cmd_helm", helm);
    m_actual_cap = m_wind_origin + helm;
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

    double prev_lat = 0;
    m_db->getData("Latitude", prev_lat);
    double prev_lng = 0;
    m_db->getData("Longitude", prev_lng);
    float course = 0.0;
    if(prev_lat != 0 && prev_lng != 0) course = get_course(prev_lat, prev_lng, m_actual_position.lat, m_actual_position.lng);

    m_db->setData("Latitude", m_actual_position.lat);
    m_db->setData("Longitude", m_actual_position.lng);
    m_db->setData("Fix_age", unsigned(0));

    unsigned time = millis()/1000;
    int second = time % 60;
    int min = ((time - second)/60) % 60;
    int hour = (time - second - min*60) / 3600;
    time = String(String(hour)+(min<10 ? "0" : "")+String(min)+(second<10 ? "0" : "")+String(second)).toInt();
    
    m_db->setData("Time", time);
    m_db->setData("Date", unsigned(200101));
    m_db->setData("Speed", msToKnots(m_actual_speed));
    m_db->setData("Course", course);
    m_db->setData("Average_course", averageCourse(course));
    m_db->setData("HDOP", int(42));
    m_db->setData("Gps_ready", true);
    m_db->setData("Fix", true);
  }

  float getWindDirection(){
    return m_wind_origin;
  }
  void setWindDirection(float wind_dir){
    m_wind_origin = wind_dir;
    from0to360(m_wind_origin);
  }
  void moveWindDirection(float diff){
    m_wind_origin = m_wind_origin + diff;
    from0to360(m_wind_origin);
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

  float m_wind_origin;
  float m_actual_cap;
  float m_actual_speed;
  Coord m_actual_position;

  AverageAngle m_course_average;

  void calculateSpeed(float wing_angle){
    
    float wind_direction = m_wind_origin + 180;
    float sail_angle = wind_direction - (wing_angle - 90);
    
    float lift_angle = (wind_direction - sail_angle) > 0 ? sail_angle+90 : sail_angle-90;
    from0to360(lift_angle);

    float diff_angle = abs(m_actual_cap - lift_angle);
    float prop_coeff = cos(radians(diff_angle));
    
    float max_speed = 5;
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
  
  float averageCourse(float new_course){
    float average_course = m_course_average.average(new_course);
    from0to360(average_course);

    m_db->setData("Average_course_full", true);

    return average_course;
  }

};

