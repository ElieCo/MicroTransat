

#undef GetData
#undef GetConf
#define GetData m_main_data->helm_manager
#define GetConf m_main_conf->helm_manager

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {

    m_last_time = -1;

    m_servo.init(7, GetConf.ratio, GetConf.offset);
  }

  void go(){

    float angle_speed;
    if (abs(GetHelmData.angle) <= GetCaptainConf.max_upwind) angle_speed = GetConf.tack_speed;
    else angle_speed = GetConf.normal_speed;

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

  int m_last_time;

  ServoMotor m_servo;

  void cmd_helm(){
    // Command the servo-motor.
    m_servo.write(GetHelmData.angle);
  }
};
