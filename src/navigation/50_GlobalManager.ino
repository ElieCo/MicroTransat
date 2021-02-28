
class GlobalManager {

public:
  GlobalManager()
    : m_configMan("ConfigManager", 10000)
    , m_missionMan("MissionManager", 1000)
    , m_logMan("LogManager", 250)
    , m_helmMan("HelmManager", 250)
    , m_wingMan("WingManager", 250)
    , m_commMan("CommManager", 0)
    , m_sensorsMan("SensorsManager", 0)
    , m_lightMan("LightManager", 500)
    , m_captain("Captain", 5000)
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

    for (int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->setDB(&m_db);
    }

    m_configMan.init();

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

  DataBase* getDB(){
    return &m_db;
  }

private:
  DataBase m_db;

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
