#include <Arduino_JSON.h>

#undef GetData
#undef GetConf
#define GetData m_main_data->conf_manager
#define GetConf m_main_conf->conf_manager

class ConfigManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(ConfigManager)
  ~ConfigManager(){}

  void init(){}

  void go(){}

  void stop(){}

  bool parseConfig(){
    GetLogData.sd_ready = m_config_file.init("bin_conf.txt", GetLogData.sd_ready);
    
    String text = m_config_file.readAll();
    pb_istream_t stream = pb_istream_from_buffer((const unsigned char*)(text.c_str()), text.length());
    
    if (!pb_decode(&stream, Conf_fields, m_main_conf)){
      print("Error reading the configuration !");
      return false;
    }
    
    return true;
  }

  private:
  SDfile m_config_file;
};
