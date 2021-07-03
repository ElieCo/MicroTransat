
#define BASIC_CONSTRUCTOR(className) className(String name, unsigned long inter = 1000) : BaseManager(name, inter){}

#define GetData m_db->data
#define GetMissionData m_db->data.mission_manager
#define GetLogData m_db->data.log_manager
#define GetHelmData m_db->data.helm_manager
#define GetWingData m_db->data.wing_manager
#define GetCaptainData m_db->data.captain_manager
#define GetCommData m_db->data.comm_manager
#define GetSensorData m_db->data.sensor_manager

#define GetConf m_db->conf
#define GetMissionConf m_db->conf.mission_manager
#define GetLogConf m_db->conf.log_manager
#define GetHelmConf m_db->conf.helm_manager
#define GetWingConf m_db->conf.wing_manager
#define GetCaptainConf m_db->conf.captain_manager
#define GetCommConf m_db->conf.comm_manager
#define GetSensorConf m_db->conf.sensor_manager

class BaseManager{

 public:

  BaseManager(String name = "BaseManager", unsigned long inter = 1000){
    m_runInterval = inter;
    m_timer = millis() - m_runInterval;
    m_name = name;
  }
  ~BaseManager(){}

  void baseInit(){
    print("Init manager:", m_name);
    
    double inter = m_runInterval;
    m_db->getData(m_name, inter);
    m_runInterval = inter;
    print("    interval:", inter);

    config();
    
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

  void initInterval(unsigned long interval){
    m_runInterval = interval;
  }

  void setDB(DataBase * db){
    m_db = db;
  }

  protected:

  String m_name;
  unsigned long m_timer;
  unsigned long m_runInterval;
  DataBase * m_db;
  
  private:

  virtual void config(){}
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
