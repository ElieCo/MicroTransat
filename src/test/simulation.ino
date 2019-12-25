
float simu_wind_direction;
float simu_dt;

void initSimuMove(float wind, float regulation, int dt){
  // wind in degrees
  // regulation in degrees
  // dt in millisecond
  simu_wind_direction = wind;
  simu_dt = dt;
  angle_regulateur = regulation;
}

void simuMove(){
  //simulate the movement and redefine the cap, speed, lat and lon
  
  speed = 5.f;
  course = float(simu_wind_direction + angle_regulateur);
  float distance = speed * simu_dt / 1000;
  float nlat, nlon;
  get_new_point(lat, lon, course, distance, nlat, nlon);
}

