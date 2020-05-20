

class PwmReader {
public:
  PwmReader(){}
  ~PwmReader(){}
  
  void init(int pin, int min_average = 55, int max_average = 95, int precision = 1000){
    pinMode(pin, INPUT);
    m_pin = pin;
    m_min_average = min_average;
    m_max_average = max_average;
    m_average.init(precision);
  }

  float updateValue(){
    float val = m_average.average(analogRead(m_pin));
    float pct = 100 * (val - m_min_average)/(m_max_average - m_min_average);
    pct = pct < 0 ? 0 : pct;
    pct = pct > 100 ? 100 : pct;
    return pct;
  }

private:
  int m_pin;
  float m_min_average;
  float m_max_average;
  Average<int> m_average;
};
