#define serialDebug Serial1

void initSerialDebug(){
  serialDebug.begin(9600);
}

void print(String s){
  serialDebug.println(s);
}

void print(String s1, String s2){
  serialDebug.print(s1);
  serialDebug.print(" ");
  serialDebug.println(s2);
}

float from0to360(float &angle){
  return limit_angle(angle, 360.0);
}

float from180to180(float &angle){
  return limit_angle(angle, 180.0);
}

float limit_angle(float &angle, float ma){

  float mi = ma - 360;
  while(angle > ma) angle -= 360;
  while(angle < mi) angle += 360;
  return angle;
}
