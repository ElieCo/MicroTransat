#include <Servo.h>

class ServoMotor
{
 public:

  ServoMotor(){}
  ~ServoMotor(){
    m_servo.detach();
  }

  void init(int pin, float ratio = 1, int offset = 0){
    m_pin = pin;
    m_ratio = ratio;
    m_offset = offset;
    m_servo.attach(m_pin);
  }

  void write(int angle){
    m_servo.write(float(angle)* m_ratio + m_offset);
  }

 protected:

  Servo m_servo;
  int m_pin;
  float m_ratio;
  int m_offset;
};
