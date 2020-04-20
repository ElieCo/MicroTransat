


class Battery
{
  public:

  Battery(){}
  ~Battery(){}

  void init(int pin){
    pinMode(pin, INPUT);
    m_pin = pin;
  }

  float getValue(){
    int value = analogRead(m_pin);
    float input_voltage = double(value) * 3.3 / 1023;
    float battery_voltage = input_voltage * (1.5 + 4.7) / 1.5;
    return battery_voltage;
  }

  private:
  int m_pin;

};
