


class Battery
{
  public:

  Battery(){}
  ~Battery(){}

  void init(int pin){
    pinMode(pin, INPUT);
    m_pin = pin;
  }

  double getValue(){
    int value = analogRead(m_pin);
    double input_voltage = double(value) * 5.0 / 1023;
    double battery_voltage = input_voltage * (1.5 + 4.7) / 1.5;
    return battery_voltage * 100;
  }

  private:
  int m_pin;

};
