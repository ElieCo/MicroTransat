

class Led
{
  public:

  Led();
  ~Led();
  
  void init(int pin){
    pinMode(pin, OUTPUT);
    m_pin = pin;
  }

  void on(){
    digitalWrite(m_pin, HIGH);
  }
  
  void off(){
    digitalWrite(m_pin, LOW);
  }

  private:

  int m_pin;
};

