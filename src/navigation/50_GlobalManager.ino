#include <vector>

class GlobalManager {

public:
  GlobalManager()
    : m_missionMan("MissionManager", 3000)
    , m_logMan("LoggerManager", 2000)
    , m_helmMan("HelmManager", 250)
    , m_wingMan("WingManager", 250)
    , m_commMan("CommManager", 0)
    , m_sensorsMan("SensorsManager", 1000)
    , m_lightMan("LightManager", 500)
    , m_captain("Captain", 500)
  {
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

    for (unsigned int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->setDB(&m_db);
    }

    for (unsigned int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->init();
    }
  }

  void globalRun(){
    for (unsigned int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->run();
    }
  }

  void globalStop(){
    for (unsigned int i = 0; i < m_managers.size(); i++){
      m_managers.at(i)->stop();
    }
  }

private:
  DataBase m_db;

  MissionManager m_missionMan;
  LogManager m_logMan;
  HelmManager m_helmMan;
  WingManager m_wingMan;
  CommManager m_commMan;
  SensorsManager m_sensorsMan;
  LightManager m_lightMan;
  Captain m_captain;

  vector<BaseManager*> m_managers;
};
