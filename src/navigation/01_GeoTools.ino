#include "TinyGPS.h"

struct Coord{
  double lat = 0.0;
  double lng = 0.0;
};

float get_distance(double lat1, double lng1, double lat2, double lng2){
  return TinyGPS::distance_between(lat1, lng1, lat2, lng2);
}

float get_distance(Coord coord1, Coord coord2){
  return get_distance(coord1.lat, coord1.lng, coord2.lat, coord2.lng);
}

float get_course(double lat1, double lng1, double lat2, double lng2){
  return TinyGPS::course_to(lat1, lng1, lat2, lng2);
}

float get_course(Coord coord1, Coord coord2){
  return get_course(coord1.lat, coord1.lng, coord2.lat, coord2.lng);
}

/**
 * Find the coordinates of the point thanks to an other point,
 * a distance and a bearing.
 * @param actual_lat and actual_lon the coordinates of the reference point.
 * @param bearing from the reference point to the result point.
 * @param distance between the two points.
 * @param new_lat and new_lon references on the coordinates of the result point.
 */
void getPointAtDistAndBearing(double actual_lat, double actual_lon, double distance, double bearing, double &new_lat, double &new_lon) {
  actual_lat = radians(actual_lat);
  actual_lon = radians(actual_lon);
  bearing = radians(bearing);
  float R = 6371000;
  new_lat = asin( sin(actual_lat) * cos(distance / R) + cos(actual_lat) * sin(distance / R) * cos(bearing) );
  new_lon = actual_lon + atan2( sin(bearing) * sin(distance / R) * cos(actual_lat), cos(distance / R) - sin(actual_lat) * sin(new_lat));
  new_lat = degrees(new_lat);
  new_lon = degrees(new_lon);
}

void getPointAtDistAndBearing(Coord actual_coord, double distance, double bearing, Coord &new_coord) {
  getPointAtDistAndBearing(actual_coord.lat, actual_coord.lng, distance, bearing, new_coord.lat, new_coord.lng);
}
