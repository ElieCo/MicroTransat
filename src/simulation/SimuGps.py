from utils import *
import time
import random
from datetime import datetime

class Gps():
    def __init__(self):
        self.lat = 0
        self.lng = 0
        self.altitude = 0
        self.fix = True
        self.fix_quality = 1
        self.satellites = 12
        self.fix_age = 0
        self.time = 120600
        self.date = 201209
        self.speed = 0
        self.course = 0
        self.hdop = 1

        self.freq = 1

        self.prev_update_time = 0

        self.std_dev = 0

    def updatePosition(self, position):
        azi = random.randint(0, 360)
        dist = random.gauss(0, self.std_dev)
        new_pos = position#.getPositionAt(dist, azi)

        # Update speed
        if self.prev_update_time > 0 and self.lat != 0 and self.lng != 0:
            d = new_pos.getDistanceTo(Position(self.lat, self.lng))
            dt = time.time() - self.prev_update_time
            self.speed = float(d)/dt
        self.prev_update_time = time.time()

        # Update course
        if self.lat != 0 and self.lng != 0:
            prev_pos = Position(self.lat, self.lng)
            self.course = prev_pos.getAzimuthTo(new_pos)

        # Update position
        self.lat = new_pos.lat
        self.lng = new_pos.lng

        # Update time
        self.time = int(datetime.now().strftime("%H%M%S"))
