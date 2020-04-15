
#define BASIC_CONSTRUCTOR(className) className(String name, unsigned long inter = 1000) : BaseManager(name, inter){}

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

    config();
    
    init();
  }

  /**
   * Run the function go if the last time was
   * at least *m_runInterval* ms before.
   */
  void run(){
    if (isTimeToRun()) {
      if (DEBUGLEVEL>= 1) print("Run: ",m_name);
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
