

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {
    m_db->initData("Cmd_helm",
    float());

    m_normal_angle_speed = 45;
    m_tack_angle_speed = 180;
    m_last_time = -1;

    int ratio = (1.0/2.0) * (180.0 / 170.0) * (21.0 / 35.0);
    m_servo.init(7, ratio, 180);
  }

  void go(){
    // Get the command angle.
    float helm_cmd;
    m_db->getData("Regulator_angle", helm_cmd);

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

  }

  private:

  void cmd_helm(){
    // Set in the DB the value of this angle.
    m_db->setData("Cmd_helm", m_helm_angle);
    // Command the servo-motor.
    m_servo.write(m_helm_angle);
  }

  float m_normal_angle_speed, m_tack_angle_speed; // deg/s
  float m_helm_angle;
  int m_last_time;

  ServoMotor m_servo;
};
