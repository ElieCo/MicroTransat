#include "TinyGPS.h"

struct Coord{
  float lat = 0.0;
  float lng = 0.0;
};

float get_distance(float lat1, float lng1, float lat2, float lng2){
  return TinyGPS::distance_between(lat1, lng1, lat2, lng2);
}

float get_course(float lat1, float lng1, float lat2, float lng2){
  return TinyGPS::course_to(lat1, lng1, lat2, lng2);
}

/**
 * Find the coordinates of the point thanks to an other point,
 * a distance and a bearing.
 * @param actual_lat and actual_lon the coordinates of the reference point.
 * @param bearing from the reference point to the result point.
 * @param distance between the two points.
 * @param new_lat and new_lon references on the coordinates of the result point.
 */
void get_point_at_dist_bearing(float actual_lat, float actual_lon, float bearing, float distance, float &new_lat, float &new_lon) {
  actual_lat = radians(actual_lat);
  actual_lon = radians(actual_lon);
  bearing = radians(bearing);
  float R = 6371000;
  new_lat = asin( sin(actual_lat) * cos(distance / R) + cos(actual_lat) * sin(distance / R) * cos(bearing) );
  new_lon = actual_lon + atan2( sin(bearing) * sin(distance / R) * cos(actual_lat), cos(distance / R) - sin(actual_lat) * sin(new_lat));
  new_lat = degrees(new_lat);
  new_lon = degrees(new_lon);
}
