from geographiclib.geodesic import Geodesic

class Position:
    def __init__(self, lat=0, lng=0):
        self.lat = lat
        self.lng = lng

    def copyFrom(self, position):
        self.lat = position.lat
        self.lng = position.lng

    def move(self, dist, azi):
        self.copyFrom(self.getPositionAt(dist, azi))

    def getPositionAt(self, dist, azi):
        data = Geodesic.WGS84.Direct(self.lat, self.lng, azi, dist)
        return Position(data["lat2"], data["lon2"])

    def getDistanceTo(self, position):
        data = Geodesic.WGS84.Inverse(self.lat, self.lng, position.lat, position.lng)
        return data["s12"]

    def getAzimuthTo(self, position):
        data = Geodesic.WGS84.Inverse(self.lat, self.lng, position.lat, position.lng)
        return data["azi1"]

class Servo:
    def __init__(self, pin, angle=0):
        self.pin = pin
        self.angle = angle

def findInBytes(sentence, pattern):
    for i in range(len(sentence) - len(pattern)):
        found = True
        for j in range(len(pattern)):
            if sentence[i+j] != pattern[j]:
                found = False
        if found:
            return i
    return -1

def knotToMs(k):
    return k*0.5144
