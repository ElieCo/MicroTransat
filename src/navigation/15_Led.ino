

class Led
{
  public:

  Led();
  ~Led();
  
  void init(int pin){
    pinMode(pin, OUTPUT);
    m_pin = pin;
    m_state = false;
  }

  void on(){
    digitalWrite(m_pin, HIGH);
    m_state = true;
  }
  
  void off(){
    digitalWrite(m_pin, LOW);
    m_state = false;
  }

  void reverse(){
    if (m_state) digitalWrite(m_pin, LOW);
    else digitalWrite(m_pin, HIGH);
    m_state = !m_state;
  }

  private:

  bool m_state;
  int m_pin;
};

