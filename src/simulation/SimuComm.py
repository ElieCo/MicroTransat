from SimuGps import Gps
from utils import *

import serial
import time
import struct

START = b'\xf1'
STOP  = b'\xf3'

class Comm:
    def __init__(self, port="/dev/ttyACM0"):
        self.servos = []
        self.serial = serial.Serial(port, 115200, timeout=0.01)

        self.buffer = b''
        self.last_gps_send = 0

    def init(self):
        t0 = time.time()
        while time.time() - t0 < 10:
            if self.read():
                self.sendGps(Gps())
                print("Serial port OK.")
                return True
        return False

    def read(self):
        data = self.serial.read(250)
        self.buffer += data
        if len(data) and data[0] != START[0]:
            msg = str(data).replace("\\r", "").replace("b'", "").replace("'", "").replace("\\n", "\n")
            print(msg, end="")

        return self.decryptBuffer()

    def decryptBuffer(self):
        start_i = findInBytes(self.buffer, START)
        stop_i = findInBytes(self.buffer, STOP)

        if start_i != -1 and stop_i != -1:
            data = self.buffer[start_i+len(START):stop_i]
            nb_servo = int(len(data) / (4+4))

            if nb_servo > 0 and nb_servo*(4+4) == len(data):
                fmt = "<" + "if" * nb_servo
                s = struct.unpack(fmt, data)
                # print(s)

                self.servos = []
                for i in range(nb_servo):
                    self.servos.append(Servo(s[i*2], s[i*2 + 1]))

            self.buffer = b''

            return True

        return False

    def sendGps(self, gps):
        if self.last_gps_send == 0 or time.time() - self.last_gps_send >= 1.0/gps.freq:
            self.last_gps_send = time.time()

            data = struct.pack("<ddf?HHIIIffi", gps.lat, gps.lng, gps.altitude, \
                                gps.fix, gps.fix_quality, gps.satellites, gps.fix_age, \
                                gps.time, gps.date, gps.speed, gps.course, gps.hdop)
            data = START + data + STOP
            self.serial.write(data)
