

#undef GetData
#undef GetConf
#define GetData m_main_data->sensor_manager
#define GetConf m_main_conf->sensor_manager

class SensorsManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(SensorsManager)
  ~SensorsManager(){}

  void init(){
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
    GetSensorData.gps.recent_data = m_gps.updateGpsData();

    if (GetSensorData.gps.recent_data){
      GetSensorData.gps.coord.latitude = m_gps.lat;
      GetSensorData.gps.coord.longitude = m_gps.lng;
      GetSensorData.gps.fix = m_gps.fix;
      GetSensorData.gps.fix_quality = m_gps.fix_quality;
      GetSensorData.gps.satellites = m_gps.satellites;
      GetSensorData.gps.fix_age = int(m_gps.fix_age);
      GetSensorData.gps.time = int(m_gps.time);
      GetSensorData.gps.date = int(m_gps.date);
      GetSensorData.gps.speed = m_gps.speed;
      GetSensorData.gps.course = m_gps.course;
      GetSensorData.gps.average_course = averageCourse(m_gps.course);
      GetSensorData.gps.hdop = m_gps.hdop;
      if (m_gps.fix && m_gps.hdop > 0 && m_gps.hdop < GetConf.gps.max_valid_hdop) GetSensorData.gps.ready = true;
      else GetSensorData.gps.ready = false;
    }

    // Manage battery
    float bat_val = m_bat.getValue();
    GetSensorData.battery.voltage = bat_val;

    if (isTime(period, 1000)){
      // Manage pwm reader
      float val = m_pwm_reader.updateValue();
      if (val != -1){
        if (val < 40) GetSensorData.radio.radio_controlled = true;
        else if (val > 60) GetSensorData.radio.radio_controlled = false;
      } else {
        GetSensorData.radio.radio_controlled = false;
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

  float averageCourse(float new_course){
    static bool prev_just_wake_up_value = GetCaptainData.just_wake_up;
    if (prev_just_wake_up_value != GetCaptainData.just_wake_up && GetCaptainData.just_wake_up) {
      GetSensorData.gps.average_course_full = false;
      m_course_average.clear();
    }
    prev_just_wake_up_value = GetCaptainData.just_wake_up;

    float average_course = m_course_average.average(new_course);
    from180to180(average_course);

    if(m_course_average.isFull()) GetSensorData.gps.average_course_full = true;

    return average_course;
  }

  Gps m_gps;
  AverageAngle m_course_average;

  Battery m_bat;

  Average<float> m_pwm_average;
  PwmReader m_pwm_reader;

  int m_last_tick = 0;
};
