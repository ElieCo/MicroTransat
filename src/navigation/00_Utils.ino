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

/**
 * Move the angle between 0 and 360 deg.
 * @param angle a reference to the angle.
 * @return angle the new angle.
 */
float from0to360(float &angle){
  return limit_angle(angle, 360.0);
}

/**
 * Move the angle between -180 and 180 deg.
 * @param angle a reference to the angle.
 * @return angle the new angle.
 */
float from180to180(float &angle){
  return limit_angle(angle, 180.0);
}

/**
 * Move the angle in a interval of 360 deg.
 * @param angle a reference to the angle.
 * @param ma the superior limit.
 * @return the new angle.
 */
float limit_angle(float &angle, float ma){
  float mi = ma - 360;
  while(angle > ma) angle -= 360;
  while(angle < mi) angle += 360;
  return angle;
}
