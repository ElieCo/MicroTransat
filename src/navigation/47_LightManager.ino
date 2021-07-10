

#undef GetData
#undef GetConf
#define GetData m_main_data->light_manager
#define GetConf m_main_conf->light_manager

class LightManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LightManager)
  ~LightManager(){}

  void init(){
    m_btn_led.init(13);
    //m_nav_led.init(???);
    //m_nav_led.on();
  }

  void go(){
    // Blink faster if we are in emergency
    if (m_emergency)
      m_runInterval = 0;
    
    // Led on if the gps is ready.
    // Led binking if not.
    if (GetSensorData.gps.ready && !m_emergency) {
      m_btn_led.on();
    } else {
      m_btn_led.reverse();
    }
  }

  void stop(){}

  void alert(){
    m_emergency = true;
  }

  private:

  Led m_btn_led;
  Led m_nav_led;
  bool m_emergency = false;
};
