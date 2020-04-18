

class SensorsManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(SensorsManager)
  ~SensorsManager(){}

  void init(){
    db_gps_recent_data.init(m_db, "Gps_recent_data", false);
    db_latitude.init(m_db, "Latitude", double(0), true);
    db_longitude.init(m_db, "Longitude", double(0), true);
    db_fix.init(m_db, "Fix", false, true);
    db_fix_quality.init(m_db, "Fix_quality", int(0));
    db_satellites.init(m_db, "Satellites", int(0));
    db_fix_age.init(m_db, "Fix_age", unsigned(0));
    db_time.init(m_db, "Time", unsigned(0));
    db_date.init(m_db, "Date", unsigned(0));
    db_speed.init(m_db, "Speed", float(0), true);
    db_course.init(m_db, "Course", float(0), true);
    db_average_course.init(m_db, "Average_course", float(0), true);
    db_hdop.init(m_db, "HDOP", int(0), true);
    db_gps_ready.init(m_db, "Gps_ready", false, true);
    db_battery.init(m_db, "Battery", float(0), true);

    // Initialize attributs needed to make the course average.
    m_x_course.init(10);
    m_y_course.init(10);

    // Initialize the battery.
    m_bat.init(A17);
  }

  void go(){

    // Manage GPS
    db_gps_recent_data.set(m_gps.updateGpsData());

    if (db_gps_recent_data.get()){
      db_latitude.set(m_gps.lat);
      db_longitude.set(m_gps.lng);
      db_fix.set(m_gps.fix);
      db_fix_quality.set(m_gps.fix_quality);
      db_satellites.set(m_gps.satellites);
      db_fix_age.set(int(m_gps.fix_age));
      db_time.set(int(m_gps.time));
      db_date.set(int(m_gps.date));
      db_speed.set(m_gps.speed);
      db_course.set(m_gps.course);
      db_average_course.set(averageCourse(m_gps.course));
      db_hdop.set(int(m_gps.hdop));
      if (m_gps.fix_quality > 0 && m_gps.hdop > 0 && m_gps.hdop < 500) db_gps_ready.set(true);
      else db_gps_ready.set(false);
    }

    // Manage battery
    float bat_val = m_bat.getValue();
    db_battery.set(bat_val);
  }

  void stop(){}

  private:

  DBData<bool> db_gps_recent_data;
  DBData<double> db_latitude;
  DBData<double> db_longitude;
  DBData<bool> db_fix;
  DBData<int> db_fix_quality;
  DBData<int> db_satellites;
  DBData<unsigned> db_fix_age;
  DBData<unsigned> db_time;
  DBData<unsigned> db_date;
  DBData<float> db_speed;
  DBData<float> db_course;
  DBData<float> db_average_course;
  DBData<int> db_hdop;
  DBData<bool> db_gps_ready;
  DBData<double> db_battery;

  float averageCourse(float new_course){
    float x_cap = m_x_course.average(float(cos(radians(new_course))));
    float y_cap = m_y_course.average(float(sin(radians(new_course))));
    
    float average_course = degrees(atan2(y_cap, x_cap));
    from180to180(average_course);

    return average_course;
  }

  Gps m_gps;
  Average<float> m_x_course;
  Average<float> m_y_course;

  Battery m_bat;
};
