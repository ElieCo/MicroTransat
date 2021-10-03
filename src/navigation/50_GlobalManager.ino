
class GlobalManager {

public:
  GlobalManager()
    : m_configMan(&m_data, &m_conf, "ConfigManager")
    , m_missionMan(&m_data, &m_conf, "MissionManager")
    , m_logMan(&m_data, &m_conf, "LogManager")
    , m_helmMan(&m_data, &m_conf, "HelmManager")
    , m_wingMan(&m_data, &m_conf, "WingManager")
    , m_commMan(&m_data, &m_conf, "CommManager")
    , m_sensorsMan(&m_data, &m_conf, "SensorsManager")
    , m_lightMan(&m_data, &m_conf, "LightManager")
    , m_captain(&m_data, &m_conf, "Captain")
  {
    m_managers.push_back(&m_configMan);
    m_managers.push_back(&m_missionMan);
    m_managers.push_back(&m_logMan);
    m_managers.push_back(&m_helmMan);
    m_managers.push_back(&m_wingMan);
    m_managers.push_back(&m_commMan);
    m_managers.push_back(&m_sensorsMan);
    m_managers.push_back(&m_lightMan);
    m_managers.push_back(&m_captain);
  }

  void globalInit(){
    initSerialDebug();

    print("=====================================");
    print("=========== Hello World! ============");
    print("=====================================");

    // Load the Conf
    if (!m_configMan.parseConfig())
      m_lightMan.alert();

    // Set the interval of each manager
    m_configMan.setInterval(m_conf.config_manager.base.interval);
    m_missionMan.setInterval(m_conf.mission_manager.base.interval);
    m_logMan.setInterval(m_conf.log_manager.base.interval);
    m_helmMan.setInterval(m_conf.helm_manager.base.interval);
    m_wingMan.setInterval(m_conf.wing_manager.base.interval);
    m_commMan.setInterval(m_conf.comm_manager.base.interval);
    m_sensorsMan.setInterval(m_conf.sensor_manager.base.interval);
    m_lightMan.setInterval(m_conf.light_manager.base.interval);
    m_captain.setInterval(m_conf.captain_manager.base.interval);

    for (int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->baseInit();
    }
  }

  void globalRun(){
    for (int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->run();
    }
  }

  void globalStop(){
    for (int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->baseStop();
    }
  }

private:
  Data m_data;
  Conf m_conf;

  ConfigManager m_configMan;

  MissionManager m_missionMan;
  LogManager m_logMan;
  HelmManager m_helmMan;
  WingManager m_wingMan;
  CommManager m_commMan;
  SensorsManager m_sensorsMan;
  LightManager m_lightMan;
  Captain m_captain;

  Vector<BaseManager*> m_managers;
};
