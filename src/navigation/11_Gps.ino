// Libraries GPS
#include <Adafruit_GPS.h>
#define GPS_SERIAL Serial5
Adafruit_GPS GPS(&GPS_SERIAL);
#define GPSECHO false

class Gps
{
  public:
  Gps()
    : lat(0)
    , lng(0)
    , altitude(0)
    , fix(false)
    , fix_quality(0)
    , satellites(0)
    , fix_age(0)
    , time(0)
    , date(0)
    , speed(0)
    , course(0)
    , hdop(0)
  {
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS.sendCommand(PGCMD_ANTENNA);
  }
  ~Gps(){}

  bool updateGpsData(){
    if (simuComm.inSimulation())
      return updateSimuGpsData();
    else
      return updateRealGpsData();
  }

  bool updateRealGpsData(){

    bool has_new_data = false;

    char c = GPS.read();

    if(c && GPSECHO) print(c);

    if (GPS.newNMEAreceived()) {
      if (GPS.parse(GPS.lastNMEA())) {

        lat = GPS.latitudeDegrees;
        lng = GPS.longitudeDegrees;
        altitude = GPS.altitude;
        fix = GPS.fix;
        fix_quality = GPS.fixquality_3d;
        satellites = GPS.satellites;
        time = getTime(GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
        date = getDate(GPS.year, GPS.month, GPS.day);
        if (fix_age == 0 && fix) fix_age = time;
        if (!fix) fix_age = 0;
        speed = GPS.speed;
        course = GPS.angle;
        hdop = GPS.HDOP * 100;

        has_new_data = true;

      }
    }
    return has_new_data;
  }

  bool updateSimuGpsData(){
    SimuComm::GpsData gps = simuComm.simuGps();

    bool new_data = time != gps.time;

    lat = gps.lat;
    lng = gps.lng;
    altitude = gps.altitude;
    fix = gps.fix;
    fix_quality = gps.fix_quality;
    satellites = gps.satellites;
    fix_age = gps.fix_age;
    time = gps.time;
    date = gps.date;
    speed = gps.speed;
    course = gps.course;
    hdop = gps.hdop;

    return new_data;
  }

  double lat, lng;
  float altitude;
  bool fix;
  int fix_quality;
  int satellites;
  unsigned fix_age, time, date;
  float speed, course;
  double hdop;

  private:

  unsigned getTime(int hour, int min, int second, int millis){

    String h = String(hour < 10 ? "0" : "") + String(hour);
    String m = String(min < 10 ? "0" : "") + String(min);
    String s = String(second < 10 ? "0" : "") + String(second);
    String ms = "";//String(millis < 100 ? "0" : "") + String(millis < 10 ? "0" : "") + String(millis);
    String time = h + m + s + ms;

    return unsigned(time.toInt());
  }


  unsigned getDate(int year, int month, int day){

    String y = String(year);
    String m = String(month < 10 ? "0" : "") + String(month);
    String d = String(day < 10 ? "0" : "") + String(day);
    String date = y + m + d;

    return unsigned(date.toInt());
  }

};
