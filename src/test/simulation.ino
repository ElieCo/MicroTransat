
float simu_wind_direction;
float simu_dt;
float simu_waypoint_lat, simu_waypoint_lon = 0;

void debug(String s){
  Serial1.println(s);
}

void initSimuMove(float wind, float regulation, int dt){
  // wind in degrees
  // regulation in degrees
  // dt in millisecond
  simu_wind_direction = wind;
  simu_dt = dt;
  angle_regulateur = regulation;
}

void initSimuPos(float _lat, float _lon){
  lat = _lat*1000000;
  lon = _lon*1000000;
  wp_lat[0] = _lat;
  wp_lon[0] = _lon;
}

void initSimuWaypoint(float wlat, float wlon){
  simu_waypoint_lat = wlat;
  simu_waypoint_lon = wlon;
  wp_lat[1] = wlat;
  wp_lon[1] = wlon;
}

void simuMove(){
  //simulate the movement and redefine the cap, speed, lat and lon
  
  
  datalog("Vitesse",(int)(speed));
  datalog("Cap",(int) course);
  datalog("Latittude",(int)(lat));
  datalog("Longitude",(int)(lon));

  
  speed = 0.2;
  course = float(simu_wind_direction + angle_regulateur);
  if(course >= 360) course -= 360;
  float distance = speed * simu_dt / 1000;
  float nlat, nlon;
  get_new_point(lat/1000000, lon/1000000, course, distance, nlat, nlon);
  lat = nlat*1000000;
  lon = nlon*1000000;


  Serial1.print("lat: ");
  Serial1.println(lat);
  Serial1.print("lon: ");
  Serial1.println(lon);
  Serial1.print("course: ");
  Serial1.println(course);
  Serial1.print("speed: ");
  Serial1.println(speed);
  Serial1.println("====================");
  
}

float getSimuAngleToWaypoint(){
  return TinyGPS::course_to(lat/1000000, lon/1000000, simu_waypoint_lat, simu_waypoint_lon);
}


float getSimuDistanceToWaypoint(){
  return TinyGPS::distance_between(lat/1000000, lon/1000000, simu_waypoint_lat, simu_waypoint_lon);
}

