

class LightManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LightManager)
  ~LightManager(){}

  void init(){
    m_btn_led.init(13);
    m_nav_led.init(39);
    m_nav_led.on();
  }

  void go(){
    // Led on if the gps is ready.
    // Led binking if not.
    bool gps_ready;
    bool has_data = m_db->getData("Gps_ready", gps_ready);
    if (has_data && gps_ready && false) {
      m_btn_led.on();
    } else {
      m_btn_led.reverse();
    }
  }

  void stop(){}

  private:

  Led m_btn_led;
  Led m_nav_led;
};
