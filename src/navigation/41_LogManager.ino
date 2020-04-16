

class LogManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LogManager)
  ~LogManager(){}

  void init(){
    m_db->initData("SD_ready", false);

    // Initialize the SD card.
    bool sd_ready = false;
    m_db->getData("SD_ready", sd_ready);
    sd_ready = m_log_csv_file.init("log.csv", sd_ready, false);
    m_db->setData("SD_ready", sd_ready);

    m_not_initialized = true;
  }

  void go(){
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
};
