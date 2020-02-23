// Libraries GPS
#include "TinyGPS.h"
#define GPS_SERIAL Serial2

class Gps
{
  public:
  Gps(){
    GPS_SERIAL.begin(9600);
  }
  ~Gps(){}

  void updateGpsData(){
    if(GPS_SERIAL.available()){
  
      char c = GPS_SERIAL.read();
      if (m_gps.encode(c))
      {
        // process new gps info here
  
        // retrieves +/- lat/long in 1000000ths of a degree
        m_gps.get_position(&lat, &lon, &fix_age);
        
        // time in hhmmsscc, date in ddmmyy
        m_gps.get_datetime(&date, &time, &fix_age);
        
        // returns speed in 100ths of a knot
        speed = m_gps.speed()/100;
        
        // course in 100ths of a degree
        course = float(m_gps.course())/100;
  
        hdop = m_gps.hdop();
      }
    }
  }

  long lat, lon;
  unsigned long fix_age, time, date;
  float speed, course;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  unsigned long hdop;

  private:
  TinyGPS m_gps;
  
};

