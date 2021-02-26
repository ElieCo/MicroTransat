from SimuGps import Gps
from SimuEasyMover import Mover
from SimuComm import Comm
from SimuDisplay import Display
from utils import *

import time

INIT_LAT = 47.731400
INIT_LNG = -3.393403

class Manager:
    def __init__(self, dt=0.1):
        self.dt = dt
        self.gps = Gps()
        self.mover = Mover()
        self.comm = Comm()
        self.display = Display(INIT_LAT, INIT_LNG)

        self.last_time_step = time.time() - self.dt

    def runSimulation(self):
        if self.init():
            while True:
                # always check communication
                self.communicate()

                # every dt seconds make a step
                if time.time() - self.last_time_step >= self.dt:
                    self.last_time_step = time.time()
                    self.step()

        else:
            print("Fail to init the simulation")

    def init(self):
        self.mover.initPosition(INIT_LAT, INIT_LNG)
        self.mover.initWind(0)

        return self.comm.init()

    def step(self):
        self.mover.step(self.dt)
        self.gps.updatePosition(self.mover.position)
        self.display.updatePosition(self.mover.position, self.gps.course)
        self.comm.sendGps(self.gps)

    def communicate(self):
        if self.comm.read():
            for s in self.comm.servos:
                self.mover.updateServo(s)
                self.display.updateServo(s)
