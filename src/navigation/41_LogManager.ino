

class LogManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LogManager)
  ~LogManager(){}

  void init(){
    

    m_not_initialized = true;
  }

  void go(){
    if (!prepareLogFile()) return;
    
    // If the first line is not initialized.
    if (m_not_initialized) {
      // Get all the data names.
      Map<String, String> data = m_db->getAllData(DEBUGLEVEL == 0);
      String line = "";
      for (int i = 0; i < data.size(); i++){
        line += data.keyAt(i).c_str();
        if (i != data.size()-1) {
          line += ";";
        }
      }
      // Log a first line all the names.
      m_log_csv_file.write(line.replace("\n","").replace("\r",""));

      // Remember that we did it.
      m_not_initialized = false;

    }
    // If we have already writen the first line.
    else {

      // Get all data.
      Map<String, String> data = m_db->getAllData(DEBUGLEVEL == 0);
      // Make a string line with all this data.
      String line = "";
      for (int i = 0; i < data.size(); i++){
        line += data.valueAt(i).c_str();
        if (i != data.size()-1) {
          line += ";";
        }
      }
      // Log this line.
      m_log_csv_file.write(line);
    }
  }

  void stop() {
    m_log_csv_file.close();
  }

  private:

  SDfile m_log_csv_file;
  bool m_not_initialized;

  bool prepareLogFile(){
    if (m_log_csv_file.isOpen()) return true;

    bool fix = false;
    m_db->getData("Fix", fix);
    unsigned time = 0;
    m_db->getData("Time", time);
    unsigned date = 0;
    m_db->getData("Date", date);

    if(fix && time != 0 && date != 0){
      String t = String(time);
      // If there is only one digit for the hour (without or with millisecond).
      if(t.length() == 5 || t.length() == 8) t = "0" + t;
      String d = String(date);
      String filename = d.substring(d.length()-4) + t.substring(0,4) + ".csv";

      // Initialize the SD card.
      bool *sd_ready = m_db->initData("SD_ready", false);
      *sd_ready = m_log_csv_file.init(filename, *sd_ready, false);
      return m_log_csv_file.isOpen();
    }
    return false;
  }
};
