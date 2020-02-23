
class DataBase;

class Manager{

 public:
 
  Manager(){
    m_runInterval = 1000;
    m_timer = millis() - m_runInterval;
    m_name = "Manager";
  }
  ~Manager(){}


  void run(){
    if (isNotTimeToRun()) return;

    go();
  }

  void initInterval(unsigned long interval){
    m_runInterval = interval;
  }

  void setDB(DataBase * db){
    m_db = db;
  }

 protected:

  void go(){
    print("Manager start:", m_name);
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

