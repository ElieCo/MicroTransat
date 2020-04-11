

class SensorsManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(SensorsManager)
  ~SensorsManager(){}

  void init(){
    m_course_tab_size = sizeof(m_course_tab) / sizeof(m_course_tab[0]);
    for (int i = 0; i < m_course_tab_size; i++)
      m_course_tab[i] = 0.0;
    m_course_index = 0;

    m_bat.init(A14);
  }

  void go(){

    // Manage GPS
    bool has_data = m_gps.updateGpsData();
    m_db->setData("Gps_recent_data", has_data);

    if (has_data){
      m_db->setData("Latitude", m_gps.lat);
      m_db->setData("Longitude", m_gps.lon);
      m_db->setData("Fix_age", int(m_gps.fix_age));
      m_db->setData("Time", int(m_gps.time));
      m_db->setData("Date", int(m_gps.date));
      m_db->setData("Speed", m_gps.speed);
      m_db->setData("Course", m_gps.course);
      m_db->setData("Average_course", averageCourse(m_gps.course));
      m_db->setData("Chars", int(m_gps.chars));
      m_db->setData("Sentences", m_gps.sentences);
      m_db->setData("Failed_checksum", m_gps.failed_checksum);
      m_db->setData("HDOP", int(m_gps.hdop));
      if (m_gps.hdop > 0 && m_gps.hdop < 500) m_db->setData("Gps_ready", true);
      else m_db->setData("Gps_ready", false);
    }

    // Manage battery
    double bat_val = m_bat.getValue();

    m_db->setData("Battery", bat_val);
  }

  private:

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
