#include <Servo.h>

class ServoMotor
{
 public:
  
  ServoMotor(int pin){
    m_pin = pin;
    m_servo.attach(m_pin);
  }
  ~ServoMotor(){
    m_servo.detach();
  }

  void write(int angle){
    m_servo.write(angle);
  }
  
 protected:

  Servo m_servo;
  int m_pin;
};

