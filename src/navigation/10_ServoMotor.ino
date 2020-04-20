#include <Servo.h>

class ServoMotor
{
 public:

  ServoMotor(){}
  ~ServoMotor(){
    m_servo.detach();
  }

  void init(int pin, int ratio = 1, int offset = 0){
    m_pin = pin;
    m_ratio = ratio;
    m_offset = offset;
    m_servo.attach(m_pin);
  }

  void write(int angle){
    m_servo.write((angle + m_offset) * m_ratio);
  }

 protected:

  Servo m_servo;
  int m_pin;
  int m_ratio;
  int m_offset;
};
