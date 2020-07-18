

class PwmReader {
public:
  PwmReader(){}
  ~PwmReader(){}
  
  void init(int pin, int min_dur = 900, int max_dur = 2085){
    pinMode(pin, INPUT);
    m_pin = pin;
    m_min_dur = min_dur;
    m_max_dur = max_dur;
  }

  float updateValue(){
    int duration = pulseIn(m_pin, HIGH, 100000);
    float pct = 100 * (duration - m_min_dur)/(m_max_dur - m_min_dur);
    pct = pct < 0 ? 0 : pct;
    pct = pct > 100 ? 100 : pct;
    pct = duration == 0 ? -1 : pct;
    return pct;
  }

private:
  int m_pin;
  float m_min_dur;
  float m_max_dur;
};
