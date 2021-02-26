from SimuGps import Gps
from SimuEasyMover import Mover
from SimuComm import Comm
from SimuDisplay import Display
from utils import *

import time


INIT_LAT = 47.731400
INIT_LNG = -3.393403

class ModelTester:
    def __init__(self):
        self.gps = Gps()
        self.mover = Mover()
        self.display = Display(INIT_LAT, INIT_LNG)
        self.prev_sec = -1

    def init():
        self.mover.initPosition(INIT_LAT, INIT_LNG)
        self.mover.initWind(90)

    def testSimulator(self, filename):
        f = open(filename, "r")

        lines = f.readlines()
        #
        # for i in range (len(lines[0].split(";"))):
        #     print(i, lines[0].split(";")[i], lines[1].split(";")[i])

        for line in lines[1:]:
            elements = line.split(";")

            radio_controlled = bool(int(elements[52]))
            lat = float(elements[40])
            lng = float(elements[41])
            speed = msToKnot(float(elements[14]))
            hour = elements[5]
            sec = float(hour[0:2])*3600 + float(hour[2:4])*60 + float(hour[4:])
            dt = sec - self.prev_sec
            self.prev_sec = sec
            cmd_helm = float(elements[11]) * float(elements[24]) + float(elements[25]);
            if float(elements[11]) < 0:
                cmd_wing = float(elements[30]) - float(elements[29])
            else:
                cmd_wing = float(elements[30]) + float(elements[29])

            if 0 < dt < sec:
                if radio_controlled:
                    self.mover.initPosition(lat, lng)

                else:
                    self.mover.updateServo(Servo(7, cmd_helm))
                    self.mover.updateServo(Servo(5, cmd_wing))
                    self.speed = speed
                    self.mover.step(dt)

                self.gps.updatePosition(self.mover.position)
                if radio_controlled:
                    color = "red"
                else:
                    color = "green"
                self.display.updatePosition(self.mover.position, self.gps.course, color)

                time.sleep(dt/10)
