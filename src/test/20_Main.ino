GlobalManager globalManager;
Simulator simulator;

void setup(){
  globalManager.globalInit();

  int lat = 0;
  int lng = 0;
  globalManager.getDB()->getData("Lat_next_point", lat);
  globalManager.getDB()->getData("Lon_next_point", lng);
  Coord first_pos;
  first_pos.lat = float(lat)/1000000;
  first_pos.lng = float(lng)/1000000;
  
  simulator.initSimulation(globalManager.getDB(), 0, first_pos);

  simulator.setMovementPeriod(100);
  simulator.setGpsPeriod(500);
}

void loop(){
  globalManager.globalRun();
  simulator.simulateMovement();
  simulator.simulateGps();
}
