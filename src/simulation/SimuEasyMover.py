from utils import *

class Mover:
    def __init__(self):
        self.position = Position()
        self.wind_direction = 0

        self.helm_angle = 0
        self.wing_angle = 0

        self.helm_ratio = -0.56
        self.helm_offset = 90

        self.speed = 5 # knots

    def initPosition(self, lat, lng):
        self.position.lat = lat
        self.position.lng = lng

    def initWind(self, direction):
        self.wind_direction = direction

    def updateServo(self, servo):
        if servo.pin == 7:
            self.helm_angle = servo.angle
        elif servo.pin == 5:
            self.wing_angle = servo.angle

    def getYaw(self):
        real_helm_angle = (self.helm_angle - self.helm_offset) / self.helm_ratio
        return self.wind_direction + real_helm_angle

    def step(self, dt):
        # Calculate distance and azimuth
        dist = knotToMs(self.speed) * dt
        azi = self.getYaw()

        # Get new position
        self.position.move(dist, azi)
