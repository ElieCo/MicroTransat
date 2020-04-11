

class LogManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LogManager)
  ~LogManager(){}

  void init(){
    // Initialize the SD card.
    m_sd.initLog();

    // Get all the data names.
    std::map<string, string> data = m_db->getAllData();
    String line = "";
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      line += it->first.c_str();
      if (it != data.end()) {
        line += ";";
      }
    }
    // Log a first line all the names.
    m_sd.log(line);
  }

  void go(){
    // Get all data.
    std::map<string, string> data = m_db->getAllData();
    // Make a string line with all this data.
    String line = "";
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      line += it->second.c_str();
      if (it != data.end()) {
        line += ";";
      }
    }
    // Log this line.
    m_sd.log(line);
  }

  void stop() {
    m_sd.stop();
  }

  private:

  SDcard m_sd;
};
