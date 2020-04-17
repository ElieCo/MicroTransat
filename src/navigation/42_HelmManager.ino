

class HelmManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(HelmManager)
  ~HelmManager(){}

  void init() {
    db_helm_angle.init(m_db, "Cmd_helm", float(0), true);
    db_cmd_applied.init(m_db, "Cmd_helm_applied", true);

    db_reg_cmd.init(m_db, "Regulator_angle", float(0));
    db_max_upwind.init(m_db, "Max_upwind", double(0));

    m_last_time = -1;

    m_servo.init(7, m_helm_ratio, m_helm_offset);
  }

  void go(){

    float angle_speed;
    if (abs(db_helm_angle.get()) <= db_max_upwind.get()) angle_speed = m_tack_angle_speed;
    else angle_speed = m_normal_angle_speed;

    // Calcul the time this the last time.
    int time = millis();
    if (m_last_time < 0) m_last_time = time;
    float diff = time - m_last_time;
    m_last_time = time;

    // Find the new helm command angle.
    if (db_reg_cmd.get() != db_helm_angle.get()) {

      // Calcul the step in degrees to do.
      float step = angle_speed * diff/1000.0;

      if ( db_reg_cmd.get() > db_helm_angle.get() + step ) {
        db_helm_angle.add(step);
      } else if ( db_reg_cmd.get() < db_helm_angle.get() - step ) {
        db_helm_angle.add(-step);
      } else {
        db_helm_angle.set(db_reg_cmd.get());
      }

      cmd_helm();
    }

    db_cmd_applied.set(db_helm_angle.get() == db_reg_cmd.get());

  }

  void stop(){}

  private:

  void config(){
    m_db->getData("Helm_ratio", m_helm_ratio);
    m_db->getData("Helm_offset", m_helm_offset);
    m_db->getData("Helm_normal_speed", m_normal_angle_speed);
    m_db->getData("Helm_tack_speed", m_tack_angle_speed);
  }

  DBData<float> db_reg_cmd;
  DBData<double> db_max_upwind;
  DBData<float> db_helm_angle;
  DBData<bool> db_cmd_applied;

  double m_normal_angle_speed, m_tack_angle_speed; // deg/s
  double m_helm_ratio, m_helm_offset;
  int m_last_time;

  ServoMotor m_servo;

  void cmd_helm(){
    // Command the servo-motor.
    m_servo.write(db_helm_angle.get());
  }
};
