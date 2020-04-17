

class WingManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(WingManager)
  ~WingManager(){}

  void init(){
    m_db->initData("Wing_angle", float(0), true);

    m_starbord_angle = 90 + m_wing_delta;
    m_port_angle = 90 - m_wing_delta;

    m_servo.init(5);
  }

  void go(){
    // Get the angle of the helm or of the regulator if the first one doesn't exists.
    float angle_helm;
    if (!m_db->getData("Cmd_helm", angle_helm))
      if (!m_db->getData("Regulator_angle", angle_helm))
        return;

    // Function of the helm angle, choose the wing angle.
    float angle_wing;
    if (angle_helm < 0) angle_wing = m_starbord_angle;
    else angle_wing = m_port_angle;

    // Command the servo-motor.
    m_servo.write(angle_wing);
    // Set in the DB the wing angle.
    m_db->setData("Wing_angle", angle_wing);
  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Wing_delta", m_wing_delta);
  }

  int m_starbord_angle, m_port_angle;
  double m_wing_delta;
  ServoMotor m_servo;
};
