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
  return limit_angle(angle, 0.0, 360.0);
}

float from180to180(float &angle){
  return limit_angle(angle, -180.0, 180.0);
}

float limit_angle(float &angle, float mi, float ma){
  while(angle > ma) angle -= 360;
  while(angle < mi) angle += 360;
  return angle;
}

