
class BaseManager{

 public:
 
  BaseManager(String name = "BaseManager", unsigned long inter = 1000){
    m_runInterval = inter;
    m_timer = millis() - m_runInterval;
    m_name = name;
  }
  ~BaseManager(){}

  void init(){
    print("Init manager:", m_name);
  }

  void run(){
    if (isNotTimeToRun()) return;
    else go();
  }

  void stop(){
    print("Stop manager:", m_name);
  }

  void initInterval(unsigned long interval){
    m_runInterval = interval;
  }

  void setDB(DataBase * db){
    m_db = db;
  }

 protected:

  void go(){
    print("Run manager:", m_name);
  }

  bool isNotTimeToRun(){
    if(millis() - m_timer > m_runInterval){
      m_timer = millis();
      return false;
    } else {
      return true;
    }
  }

  String m_name;
  unsigned long m_timer;
  unsigned long m_runInterval;
  DataBase * m_db;
  
};

