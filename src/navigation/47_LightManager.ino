

class LightManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LightManager)
  ~LightManager(){}

  void init(){
    db_gps_ready.init(m_db, "Gps_ready", false);

    m_btn_led.init(13);
    //m_nav_led.init(???);
    //m_nav_led.on();
  }

  void go(){
    // Led on if the gps is ready.
    // Led binking if not.
    if (db_gps_ready.get()) {
      m_btn_led.on();
    } else {
      m_btn_led.reverse();
    }
  }

  void stop(){}

  private:

  DBData<bool> db_gps_ready;

  Led m_btn_led;
  Led m_nav_led;
};
