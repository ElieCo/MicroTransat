

class LogManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LogManager)
  ~LogManager(){}

  void init(){
    m_sd.initLog();

    std::map<string, string> data = m_db->getAllData();
    String line = "";
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      line += it->first.c_str();
      if (it != data.end()) {
        line += ";";
      }
    }
    m_sd.log(line);
  }

  void go(){
    std::map<string, string> data = m_db->getAllData();
    String line = "";
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      line += it->second.c_str();
      if (it != data.end()) {
        line += ";";
      }
    }
    m_sd.log(line);
  }

  void stop() {
    m_sd.stop();
  }

  private:

  SDcard m_sd;
};
