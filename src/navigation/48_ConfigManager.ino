#include <Arduino_JSON.h>


class ConfigManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(ConfigManager)
  ~ConfigManager(){}

  void init(){
    if (m_config.length() > 0) return;

    bool *sd_ready = m_db->initData("SD_ready", false);
    *sd_ready = m_config_file.init("config.txt", *sd_ready);
    parseConfig();
  }

  void go(){}

  void stop(){}

  private:
  SDfile m_config_file;
  JSONVar m_config;

  void parseConfig(){
    String text = m_config_file.readAll();

    m_config = JSON.parse(text);

    for (int i = 0; i < m_config.length(); i++){

      if (m_config[i].hasOwnProperty("name") && m_config[i].hasOwnProperty("interval")) {
        String name = (const char*) m_config[i]["name"];
        double interval = m_config[i]["interval"];
        m_db->initData(name, interval);
      }

      if (m_config[i].hasOwnProperty("parameters")){
        JSONVar parameters = m_config[i]["parameters"];

        for (int j = 0; j < parameters.length(); j++){
          if (parameters[j].hasOwnProperty("key")) {
            String key = (const char*) parameters[j]["key"];
            String type = JSON.typeof(parameters[j]["value"]);
            if (type == "number") m_db->initData(key, double(parameters[j]["value"]));
            else if (type == "boolean") m_db->initData(key, bool(parameters[j]["value"]));
            else if (type == "string") m_db->initData(key, String((const char*) parameters[j]["value"]));
          }
        }
      }
    }
  }
};
