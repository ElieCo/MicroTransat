

#undef GetData
#undef GetConf
#define GetData m_main_data->wing_manager
#define GetConf m_main_conf->wing_manager

class WingManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(WingManager)
  ~WingManager(){}

  void init(){
    m_starbord_angle =  GetConf.calib_offset - GetConf.step;
    m_port_angle =      GetConf.calib_offset + GetConf.step;

    m_servo.init(5, 1, 90);
  }

  void go(){

    // Function of the helm angle, choose the wing angle.
    if (GetHelmData.angle < 0) GetWingData.angle = m_starbord_angle;
    else GetWingData.angle = m_port_angle;

    // Command the servo-motor.
    m_servo.write(GetWingData.angle);
  }

  void stop(){}

  private:

  int m_starbord_angle, m_port_angle;
  ServoMotor m_servo;
};
