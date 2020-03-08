#include "TinyGPS.h"

float get_distance(float lat1, float lng1, float lat2, float lng2){
  return TinyGPS::distance_between(lat1, lng1, lat2, lng2);
}

float get_course(float lat1, float lng1, float lat2, float lng2){
  return TinyGPS::course_to(lat1, lng1, lat2, lng2);
}

