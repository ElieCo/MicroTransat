

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {

    db_max_upwind.init(m_db, "Max_upwind", double(0));

    m_last_time = -1;

    m_servo.init(7, m_helm_ratio, m_helm_offset);
  }

  void go(){

    float angle_speed;
    if (abs(GetHelmData.angle) <= db_max_upwind.get()) angle_speed = m_tack_angle_speed;
    else angle_speed = m_normal_angle_speed;

    // Calcul the time this the last time.
    int time = millis();
    if (m_last_time < 0) m_last_time = time;
    float diff = time - m_last_time;
    m_last_time = time;

    // Find the new helm command angle.
    if (GetCaptainData.helm_order != GetHelmData.angle) {

      // Calcul the step in degrees to do.
      float step = angle_speed * diff/1000.0;

      if (GetCaptainData.helm_order > GetHelmData.angle + step) {
        GetHelmData.angle += step;
      } else if (GetCaptainData.helm_order < GetHelmData.angle - step) {
        GetHelmData.angle -= step;
      } else {
        GetHelmData.angle = GetCaptainData.helm_order;
      }

      cmd_helm();
    }

    GetHelmData.cmd_applied = (GetHelmData.angle == GetCaptainData.helm_order);
  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Helm_ratio", m_helm_ratio);
    m_db->getData("Helm_offset", m_helm_offset);
    m_db->getData("Helm_normal_speed", m_normal_angle_speed);
    m_db->getData("Helm_tack_speed", m_tack_angle_speed);
  }

  DBData<double> db_max_upwind;

  double m_normal_angle_speed, m_tack_angle_speed; // deg/s
  double m_helm_ratio, m_helm_offset;
  int m_last_time;

  ServoMotor m_servo;

  void cmd_helm(){
    // Command the servo-motor.
    m_servo.write(GetHelmData.angle);
  }
};
