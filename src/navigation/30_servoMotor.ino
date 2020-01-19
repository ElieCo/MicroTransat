

class ServoMotor : public Manager
{
 public:
  
  ServoMotor(String name, int pin){
    m_name = name;
    m_pin = pin;
    m_servo.attach(m_pin);

    m_runInterval = 2000;
  }
  ~ServoMotor(){}

  void write(int angle){
    m_servo.write(angle);
  }
  
 protected:

  Servo m_servo;
  int m_pin;
};

