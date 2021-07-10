
#define BASIC_CONSTRUCTOR(className) className(Data *data, Conf *conf, String name, unsigned int inter = 1000) : BaseManager(data, conf, name, inter){}

#define GetMainData m_main_data
#define GetMissionData m_main_data->mission_manager
#define GetLogData m_main_data->log_manager
#define GetHelmData m_main_data->helm_manager
#define GetWingData m_main_data->wing_manager
#define GetCaptainData m_main_data->captain_manager
#define GetCommData m_main_data->comm_manager
#define GetSensorData m_main_data->sensor_manager
#define GetLightData m_main_data->light_manager
#define GetConfigData m_main_data->config_manager

#define GetMainConf m_main_conf
#define GetMissionConf m_main_conf->mission_manager
#define GetLogConf m_main_conf->log_manager
#define GetHelmConf m_main_conf->helm_manager
#define GetWingConf m_main_conf->wing_manager
#define GetCaptainConf m_main_conf->captain_manager
#define GetCommConf m_main_conf->comm_manager
#define GetSensorConf m_main_conf->sensor_manager
#define GetLightConf m_main_conf->light_manager
#define GetConfigConf m_main_conf->config_manager

class BaseManager{

 public:

  BaseManager(Data *data, Conf *conf, String name = "BaseManager", unsigned long inter = 1000){
    m_runInterval = inter;
    m_timer = millis() - m_runInterval;
    m_name = name;
    m_main_data = data;
    m_main_conf = conf;
  }
  ~BaseManager(){}

  void baseInit(){
    print("Init manager:", m_name);
    print("    interval:", m_runInterval);

    init();
  }

  /**
   * Run the function go if the last time was
   * at least *m_runInterval* ms before.
   */
  void run(){
    if (isTimeToRun()) {
      if (DEBUGLEVEL>= 2) print("Run: ",m_name);
      go();
    }
  }

  void baseStop(){
    print("Stop manager:", m_name);
    stop();
  }

  void setInterval(unsigned long inter){
    m_runInterval = inter;
  }

  protected:

  String m_name;
  unsigned long m_timer;
  unsigned long m_runInterval;
  Data *m_main_data;
  Conf *m_main_conf;
  
  private:

  virtual void init(){}
  virtual void go(){}
  virtual void stop(){}

  /**
   * Check the time between two call.
   * @return true if it was more than
   * *m_runInterval* ms ago, else false.
   */
  bool isTimeToRun(){
    if(millis() - m_timer > m_runInterval){
      m_timer = millis();
      return true;
    } else {
      return false;
    }
  }

};
