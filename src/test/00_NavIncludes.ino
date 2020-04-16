/////////////////////////////////////////////////
//                   Define
/////////////////////////////////////////////////

#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define CONCAT(x,y) IDENT(x)IDENT(y)
#define PATH(x,y) STR(IDENT(x)IDENT(y))

#define REPO /home/elie/Projets/Bateau/MicroTransat/
#define NAVREPO CONCAT(REPO,src/navigation/)

#define SIMU true


/////////////////////////////////////////////////
//          Includes and overwriting
/////////////////////////////////////////////////

#include PATH(NAVREPO,00_Utils.ino)
#include PATH(NAVREPO,01_GeoTools.ino)
#include PATH(NAVREPO,02_Map.ino)
#include PATH(NAVREPO,03_Vector.ino)

//#include PATH(NAVREPO,10_ServoMotor.ino)
class ServoMotor
{
 public:
  ServoMotor(){}
  ~ServoMotor(){}
  void init(int pin, int ratio = 1, int offset = 0){}
  void write(int angle){}
};

#include PATH(NAVREPO,11_Gps.ino)
#include PATH(NAVREPO,12_LoRa.ino)
#include PATH(NAVREPO,13_SDfile.ino)
#include PATH(NAVREPO,14_Battery.ino)
#include PATH(NAVREPO,15_Led.ino)
#include PATH(NAVREPO,20_DataBase.ino)
#include PATH(NAVREPO,30_BaseManager.ino)
#include PATH(NAVREPO,40_MissionManager.ino)
#include PATH(NAVREPO,41_LogManager.ino)
#include PATH(NAVREPO,42_HelmManager.ino)
#include PATH(NAVREPO,43_WingManager.ino)
#include PATH(NAVREPO,44_CaptainManager.ino)
#include PATH(NAVREPO,45_CommManager.ino)

//#include PATH(NAVREPO,46_SensorsManager.ino)
class SensorsManager: public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(SensorsManager)
  ~SensorsManager(){}

  void init(){
    m_db->initData("Gps_recent_data", bool());
    m_db->initData("Latitude", long(), true);
    m_db->initData("Longitude", long(), true);
    m_db->initData("Fix_age", int());
    m_db->initData("Time", int());
    m_db->initData("Date", int());
    m_db->initData("Speed", float(), true);
    m_db->initData("Course", float(), true);
    m_db->initData("Average_course", float(), true);
    m_db->initData("Chars", int());
    m_db->initData("Sentences", unsigned());
    m_db->initData("Failed_checksum", unsigned());
    m_db->initData("HDOP", int(), true);
    m_db->initData("Gps_ready", bool(), true);
    m_db->initData("Battery", double(), true);

    // Initialize the battery.
    m_bat.init(A14);
  }

  void go(){

    // Manage GPS
    // will be simulate

    // Manage battery
    double bat_val = m_bat.getValue();
    m_db->setData("Battery", bat_val);
  }

  void stop(){}

  private:
  Battery m_bat;
};

#include PATH(NAVREPO,47_LightManager.ino)
#include PATH(NAVREPO,50_GlobalManager.ino)
