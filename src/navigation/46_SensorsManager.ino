

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
    db_time.init(m_db, "Time", unsigned(0), true);
    db_date.init(m_db, "Date", unsigned(0));
    db_speed.init(m_db, "Speed", float(0), true);
    db_course.init(m_db, "Course", float(0), true);
    db_average_course.init(m_db, "Average_course", float(0), true);
    db_hdop.init(m_db, "HDOP", double(0), true);
    db_gps_ready.init(m_db, "Gps_ready", false, true);
    db_battery.init(m_db, "Battery", float(0), true);
    db_just_wake_up.init(m_db, "Just_wake_up", false);
    db_average_course_full.init(m_db, "Average_course_full", false);
    db_radio_controlled.init(m_db, "Radio_controlled", true, true);

    // Initialize the course average.
    m_course_average.init(3);

    // Initialize the battery.
    m_bat.init(A16);

    // Initialize the pwm reader.
    m_pwm_reader.init(39);
  }

  void go(){

    int period = millis() - m_last_tick;
    m_last_tick = millis();

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
      db_hdop.set(m_gps.hdop);
      if (m_gps.fix_quality > 0 && m_gps.hdop > 0 && m_gps.hdop < m_max_valid_hdop) db_gps_ready.set(true);
      else db_gps_ready.set(false);
    }

    // Manage battery
    float bat_val = m_bat.getValue();
    db_battery.set(bat_val);

    if (isTime(period, 1000)){
      // Manage pwm reader
      float val = m_pwm_reader.updateValue();
      if (val != -1){
        if (val < 40) db_radio_controlled.set(true);
        else if (val > 60) db_radio_controlled.set(false);
      }
    }
  }

  void stop(){}

  private:

  bool isTime(int manager_interval, int ms){
    int margin = manager_interval*0.9;
    int r = millis() % ms;
    if (r <= margin)
      return true;
    else
      return false;
  }

  void config(){
    m_db->getData("Max_valid_hdop", m_max_valid_hdop);
  }

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
  DBData<double> db_hdop;
  DBData<bool> db_gps_ready;
  DBData<double> db_battery;
  DBData<bool> db_just_wake_up;
  DBData<bool> db_average_course_full;
  DBData<bool> db_radio_controlled;

  float averageCourse(float new_course){
    if (db_just_wake_up.hasChanged() && db_just_wake_up.get()) {
      db_average_course_full.set(false);
      m_course_average.clear();
    }

    float average_course = m_course_average.average(new_course);
    from180to180(average_course);

    if(m_course_average.isFull()) db_average_course_full.set(true);

    return average_course;
  }

  Gps m_gps;
  AverageAngle m_course_average;
  double m_max_valid_hdop;

  Battery m_bat;

  Average<float> m_pwm_average;
  PwmReader m_pwm_reader;

  int m_last_tick = 0;
};
