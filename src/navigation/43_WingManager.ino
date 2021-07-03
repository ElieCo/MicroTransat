

class WingManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(WingManager)
  ~WingManager(){}

  void init(){
    m_starbord_angle =  m_wing_calib - m_wing_delta;
    m_port_angle =      m_wing_calib + m_wing_delta;

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

  void config(){
    m_db->getData("Wing_delta", m_wing_delta);
    m_db->getData("Wing_calib", m_wing_calib);
  }

  int m_starbord_angle, m_port_angle;
  double m_wing_delta, m_wing_calib;
  ServoMotor m_servo;
};
