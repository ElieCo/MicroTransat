

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {
    m_db->initData("Cmd_helm", float(0), true);
    m_db->initData("Cmd_helm_applied", true);

    m_last_time = -1;

    m_servo.init(7, m_helm_ratio, m_helm_offset);
  }

  void go(){
    // Get the command angle.
    float helm_cmd;
    m_db->getData("Regulator_angle", helm_cmd);

    // Choose the speed in function of the angle to the wind
    double max_upwind = 30;
    m_db->getData("Max_upwind", max_upwind);
    float angle_speed;
    if (abs(m_helm_angle) <= max_upwind) angle_speed = m_tack_angle_speed;
    else angle_speed = m_normal_angle_speed;

    // Calcul the time this the last time.
    int time = millis();
    if (m_last_time < 0) m_last_time = time;
    float diff = time - m_last_time;
    m_last_time = time;

    // Calcul the step in degrees to do.
    float step = angle_speed * diff/1000.0;

    // Find the new helm command angle.
    if (helm_cmd != m_helm_angle) {
      if ( helm_cmd > m_helm_angle + step ) {
        m_helm_angle += step;
      } else if ( helm_cmd < m_helm_angle - step ) {
        m_helm_angle -= step;
      } else {
        m_helm_angle = helm_cmd;
      }

      cmd_helm();
    }

    m_db->setData("Cmd_helm_applied", m_helm_angle == helm_cmd);

  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Helm_ratio", m_helm_ratio);
    m_db->getData("Helm_offset", m_helm_offset);
    m_db->getData("Helm_normal_speed", m_normal_angle_speed);
    m_db->getData("Helm_tack_speed", m_tack_angle_speed);
  }

  double m_normal_angle_speed, m_tack_angle_speed; // deg/s
  double m_helm_ratio, m_helm_offset;
  float m_helm_angle;
  int m_last_time;

  ServoMotor m_servo;

  void cmd_helm(){
    // Set in the DB the value of this angle.
    m_db->setData("Cmd_helm", m_helm_angle);
    // Command the servo-motor.
    m_servo.write(m_helm_angle);
  }
};
