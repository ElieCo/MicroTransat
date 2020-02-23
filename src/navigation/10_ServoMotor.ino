

class ServoMotor
{
 public:
  
  ServoMotor(int pin){
    m_pin = pin;
    m_servo.attach(m_pin);
  }
  ~ServoMotor(){}

  void write(int angle){
    m_servo.write(angle);
  }
  
 protected:

  Servo m_servo;
  int m_pin;
};

