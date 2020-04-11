

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {
    m_normal_angle_speed = 45;
    m_tack_angle_speed = 180;
    m_last_time = -1;

    m_servo.init(7);
  }

  void go(){
    float cmd;
    m_db->getData("Regulator_angle", cmd);

    float max_upwind = 30;
    m_db->getData("Max_upwind", max_upwind);
    float angle_speed;
    if (abs(m_helm_angle) <= max_upwind) angle_speed = m_tack_angle_speed;
    else angle_speed = m_normal_angle_speed;

    // TODO : put security and adaptation on the low level ? -> il faut que la vitesse corresponde a la vitesse de la gouverne et pas du servo
    float helm_cmd = (cmd+180) * (1/2) * (180.0 / 170.0) * (21.0 / 35.0);

    int time = millis();
    if (m_last_time < 0) m_last_time = time;
    float diff = time - m_last_time;
    m_last_time = time;

    float step = angle_speed * diff/1000.0;

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
    m_db->setData("Cmd_helm", m_helm_angle);
    m_servo.write(m_helm_angle);
  }

  float m_normal_angle_speed, m_tack_angle_speed; // deg/s
  float m_helm_angle;
  int m_last_time;

  ServoMotor m_servo;
};
