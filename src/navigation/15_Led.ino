

class Led
{
  public:

  Led(int pin){
    pinMode(pin, OUTPUT);
    m_pin = pin;
  }
  ~Led();

  void on(){
    digitalWrite(m_pin, HIGH);
  }
  
  void off(){
    digitalWrite(m_pin, LOW);
  }

  private:

  int m_pin;
};

