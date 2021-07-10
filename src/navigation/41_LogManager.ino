

#undef GetData
#undef GetConf
#define GetData m_main_data->log_manager
#define GetConf m_main_conf->log_manager

class LogManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(LogManager)
  ~LogManager(){}

  void init(){
    m_not_initialized = true;
  }

  void go(){
    /*size_t s;
    pb_get_encoded_size(&s, Data_fields, GetMainData);
    print("\nsize:", s);*/
    if (!prepareLogFile()) return;
    
    /*pb_get_encoded_size(&s, Data_fields, GetMainData);
    print("\nsize 2:", s);*/
    // If the conf is not saved.
    if (m_not_initialized) {
      // Log Conf
      logConf();

      // Remember that we did it.
      m_not_initialized = false;
    }
    
    /*pb_get_encoded_size(&s, Data_fields, GetMainData);
    print("\nsize 3:", s);*/
    // Always log Data
    logData();
    /*pb_get_encoded_size(&s, Data_fields, GetMainData);
    print("\nsize 4:", s);*/
  }

  void stop() {
    m_log_bin_file.close();
  }

  private:

  SDfile m_log_bin_file;
  bool m_not_initialized;

  bool prepareLogFile(){
    if (m_log_bin_file.isOpen()) return true;

    bool fix = GetSensorData.gps.fix;
    unsigned time = GetSensorData.gps.time;
    unsigned date = GetSensorData.gps.date;

    if(time != 0 && date != 0 && fix){
      String t = String(time);
      for (unsigned i = 0; i < max(unsigned(0), 6 - String(time).length()); i++) t = "0" + t;
      String d = String(date);
      String filename = d.substring(d.length()-4) + t.substring(0,4) + ".bin";

      // Initialize the SD card.
      GetLogData.sd_ready = m_log_bin_file.init(filename, GetLogData.sd_ready, false);
      return m_log_bin_file.isOpen();
    }
    return false;
  }

  void logConf(){
    size_t _size;
    pb_get_encoded_size(&_size, Conf_fields, GetMainConf);

    pb_byte_t buf[_size + 4];
    memcpy(buf, &_size, 4);
    
    pb_ostream_t stream;
    stream = pb_ostream_from_buffer(&buf[4], sizeof(buf));
    
    if (pb_encode(&stream, Conf_fields, GetMainConf))
      m_log_bin_file.write(buf, sizeof(buf));
  }

  void logData(){
    size_t _size;
    pb_get_encoded_size(&_size, Data_fields, GetMainData);

    pb_byte_t buf[_size + 4];
    memcpy(buf, &_size, 4);
    
    pb_ostream_t stream;
    stream = pb_ostream_from_buffer(&buf[4], sizeof(buf));
    
    if (pb_encode(&stream, Data_fields, GetMainData))
      m_log_bin_file.write(buf, sizeof(buf));
  }
};
