

class LightManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LightManager)
  ~LightManager(){}

  void init(){
    m_btn_led.init(13);
  }

  void go(){
    bool gps_ready;
    if (m_db->getData("Gps_ready", gps_ready)){
      if (gps_ready) {
        m_btn_led.on();
      } else {
        m_btn_led.reverse();
      }
    }
  }

  private:

  Led m_btn_led;
};
