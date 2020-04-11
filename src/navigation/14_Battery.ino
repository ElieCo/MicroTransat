


class Battery
{
  public:

  Battery(){}
  ~Battery(){}

  double getValue(){
    int value = analogRead(A14);
    double input_voltage = double(value) * 5.0 / 1023;
    double battery_voltage = input_voltage * (1.5 + 4.7) / 1.5;
    return battery_voltage * 100;
  }
};
