

class PwmReader {
public:
  PwmReader(){}
  ~PwmReader(){}
  
  void init(int pin, int min_val = 65, int max_val = 95){
    pinMode(pin, INPUT);
    m_pin = pin;
    m_min_val = min_val;
    m_max_val = max_val;
  }

  float updateValue(){
    float val = analogRead(m_pin);
    float pct = 100 * (val - m_min_val)/(m_max_val - m_min_val);
    pct = pct < 0 ? 0 : pct;
    pct = pct > 100 ? 100 : pct;
    return pct;
  }

private:
  int m_pin;
  float m_min_val;
  float m_max_val;
};
