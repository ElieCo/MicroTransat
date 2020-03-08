#include <Servo.h>

class ServoMotor
{
 public:
  
  ServoMotor();
  ~ServoMotor(){
    m_servo.detach();
  }

  void init(int pin){
    m_pin = pin;
    m_servo.attach(m_pin);
  }

  void write(int angle){
    m_servo.write(angle);
  }
  
 protected:

  Servo m_servo;
  int m_pin;
};

