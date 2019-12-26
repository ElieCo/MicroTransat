#include "/home/elie/Projets/MicroTransat/src/navigation/navigation.ino"

bool first_test_loop;

void setup(){

  navSetup();
  
  // Serial1.begin(9600);
  Serial1.println("");
  Serial1.println("======================");
  Serial1.println("=== HERE THE TESTS ===");
  Serial1.println("======================");

  first_test_loop = true;
}

void loop(){
  if(first_test_loop){
    // run tests only once
    // testNav_02();
  }

  // run tests in a loop
  // testNav_01();
  testGPS();


  first_test_loop = false;
  // delay(1000);
}

bool testNav_01(){
  timer2 = millis() + interval_calcul + 1;
  index_wpt = 0;

  if(first_test_loop){
    initSimuMove(290, 50, interval_calcul);
    initSimuPos(47.731564, -3.393134);
    initSimuWaypoint(47.732344, -3.395307);
  }
  simuMove();
  
  cap_moyen = course;
  angleToWaypoint = getSimuAngleToWaypoint();
  distanceToWaypoint = getSimuDistanceToWaypoint();

  mode_autonome();
  datalog("push", 0);

  Serial1.print("distance to waypoint: ");
  Serial1.println(distanceToWaypoint);
}

bool testNav_02(){
  float m_lat = 48.360731;
  float m_lon = -4.566752;
  float m_bearing = 27;
  float m_dist = 108.56;

  float m_new_lat, m_new_lon;
  
  get_new_point(m_lat, m_lon, m_bearing, m_dist, m_new_lat, m_new_lon);

  float m_good_lat = 48.36160087;
  float m_good_lon = -4.5660869;

  float diff = TinyGPS::distance_between(m_new_lat, m_new_lon, m_good_lat, m_good_lon);
  
  if(diff <= 1)
    return true;
  else
    return false;
}

bool testGPS(){
  
  while (Serial2.available())
  {
    //Serial1.println("available");
    int c = Serial2.read();
    if (gps.encode(c))
    {
      // process new gps info here
      long lat, lon;
      unsigned long fix_age, time, date, speed, course;
      unsigned long chars;
      unsigned short sentences, failed_checksum;
      unsigned long hdop;
      
      // retrieves +/- lat/long in 1000000ths of a degree
      gps.get_position(&lat, &lon, &fix_age);
      
      // time in hhmmsscc, date in ddmmyy
      gps.get_datetime(&date, &time, &fix_age);
      
      // returns speed in 100ths of a knot
      speed = gps.speed();
      
      // course in 100ths of a degree
      course = gps.course();

      hdop = gps.hdop();

      Serial1.println(lat);
      Serial1.println(lon);
      Serial1.println("====== PLOP ============");
    }
  }
}

