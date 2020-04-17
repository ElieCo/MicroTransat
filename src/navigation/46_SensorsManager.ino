

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
    db_battery.init(m_db, "Battery", double(0), true);

    // Initialize attributs needed to make the course average.
    m_course_tab_size = sizeof(m_course_tab) / sizeof(m_course_tab[0]);
    for (int i = 0; i < m_course_tab_size; i++)
      m_course_tab[i] = 0.0;
    m_course_index = 0;

    // Initialize the battery.
    m_bat.init(A16);
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
    double bat_val = m_bat.getValue();
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
    m_course_tab[m_course_index] = new_course;
    m_course_index = (m_course_index + 1) % m_course_tab_size;

    float x_cap = 0;
    float y_cap = 0;

    for (int i = 0; i < m_course_tab_size; i++) {
      x_cap += cos(radians(m_course_tab[i]));
      y_cap += sin(radians(m_course_tab[i]));
    }

    float average_course = atan2(y_cap, x_cap);

    return average_course;
  }

  Gps m_gps;
  float m_course_tab[10];
  int m_course_index;
  int m_course_tab_size;

  Battery m_bat;
};
