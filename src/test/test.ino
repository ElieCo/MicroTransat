#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define PATH(x,y) STR(IDENT(x)IDENT(y))

#define REPO /home/elie/Projets/MicroTransat/

#include PATH(REPO,src/navigation/navigation.ino)

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
  testNav_01();
  // testGPS();
  // testServoBar();
  // testServoWing();


  first_test_loop = false;
}

bool testNav_01(){
  smooth_bar();
  
  static int test_interval = 1000;
  static int test_timer = millis() - test_interval;
  if(millis() - test_timer < test_interval)
    return false;
  test_timer = millis();
  
  timer2 = millis() + interval_calcul + 1;

  if(first_test_loop){
    initSimuMove(290, 50, interval_calcul);
    initSimuPos(47.731775, -3.394241);
    initSimuWaypoint(47.731042, -3.393050);
  }
  simuMove();
  
  cap_moyen = course;
  angleToWaypoint = getSimuAngleToWaypoint();
  distanceToWaypoint = getSimuDistanceToWaypoint();
  if (next_point(distanceToWaypoint)){
    angleToWaypoint = getSimuAngleToWaypoint();
    distanceToWaypoint = getSimuDistanceToWaypoint();
  }

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

bool testServoBar(){
  static int test_interval = 3000;
  static int test_timer = millis() - test_interval;
  if(millis() - test_timer < test_interval)
    return false;
  test_timer = millis();
  
  static int regulation_angle = 0;
  static int step = 45;
  Serial1.print("regulation angle: ");
  Serial1.print(regulation_angle);
  double real_angle = (-regulation_angle*(180.0/170.0)*(21.0/35.0)/2)+90;
  Serial1.print("    ");
  Serial1.println(real_angle);
  barre.write(real_angle);
  regulation_angle += step;
  if(regulation_angle > 90.0 || regulation_angle < -90.0){
    step = -step;
    regulation_angle += 2*step;
  }
  
}

bool testServoWing(){
  static int test_interval = 1000;
  static int test_timer = millis() - test_interval;
  if(millis() - test_timer < test_interval)
    return false;
  test_timer = millis();
  
  static int wing_angle = 0;
  static int step = 10;
  Serial1.print("wing angle: ");
  Serial1.println(wing_angle);
  aile.write(wing_angle*(180.0/170.0));
  wing_angle += step;
  if(wing_angle > 170 || wing_angle < 0){
    step = -step;
    wing_angle += 2*step;
  }
  
}

