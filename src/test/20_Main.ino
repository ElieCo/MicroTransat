GlobalManager globalManager;
Simulator simulator;

void setup(){
  globalManager.globalInit();

  Coord first_pos;
  globalManager.getDB()->getData("Lat_next_point", first_pos.lat);
  globalManager.getDB()->getData("Lon_next_point", first_pos.lng);
  
  simulator.initSimulation(globalManager.getDB(), 100, first_pos);

  simulator.setMovementPeriod(100);
  
  double sensor_interval = 1000;
  //globalManager.getDB()->getData("SensorsManager", sensor_interval);
  simulator.setGpsPeriod(sensor_interval);
}

void loop(){
  globalManager.globalRun();
  simulator.simulateMovement();
  simulator.simulateGps();
}
