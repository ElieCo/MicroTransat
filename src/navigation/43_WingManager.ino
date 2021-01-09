

class WingManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(WingManager)
  ~WingManager(){}

  void init(){
    db_wing_angle.init(m_db, "Wing_angle", float(0), true);

    db_helm_angle.init(m_db, "Cmd_helm", float(0));

    m_starbord_angle = 90 - m_wing_delta + m_wing_calib;
    m_port_angle = 90 + m_wing_delta + m_wing_calib;

    m_servo.init(5);
  }

  void go(){

    // Function of the helm angle, choose the wing angle.
    if (db_helm_angle.get() < 0) db_wing_angle.set(m_starbord_angle);
    else db_wing_angle.set(m_port_angle);

    // Command the servo-motor.
    m_servo.write(db_wing_angle.get());
  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Wing_delta", m_wing_delta);
    m_db->getData("Wing_calib", m_wing_calib);
  }

  DBData<float> db_helm_angle;
  DBData<float> db_wing_angle;

  int m_starbord_angle, m_port_angle;
  double m_wing_delta, m_wing_calib;
  ServoMotor m_servo;
};
