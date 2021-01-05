import numpy as np
import matplotlib.pyplot as plt
from utils import *

class Display():
    def __init__(self, lat, lng):
        self.ref_pos = Position(lat, lng)

        self.d_boat = None

        delta1 = 50
        self.fig = plt.figure(figsize=[10,10])
        self.ax1 = self.fig.add_subplot(111)
        self.ax1.set_ylim(bottom=-delta1, top=delta1)
        self.ax1.set_xlim(left=-delta1, right=delta1)
        self.fig.show()

        self.prev_x = None
        self.prev_y = None

    def updatePosition(self, pos, course):
        x, y = self.geoToXY(pos)
        boat_pt = self.getLinePt(np.array([x, y]), 90-course, 1)
        self.d_boat = self.update2DLine(self.d_boat, np.array([x, y]), boat_pt)

        if self.prev_x != None and self.prev_y != None:
            self.ax1.plot([self.prev_x, x], [self.prev_y, y], color="red", linewidth=0.2)
        self.prev_x = x
        self.prev_y = y

        self.fig.canvas.draw()
        self.fig.canvas.flush_events()

    def updateServo(self, s):
        pass

    def geoToXY(self, pos):
        azi = np.radians(self.ref_pos.getAzimuthTo(pos))
        dist = self.ref_pos.getDistanceTo(pos)
        x = np.sin(azi) * dist
        y = np.cos(azi) * dist

        return (x,y)

    def getLinePt(self, pt, direction, length, scale = 1):
        direction = np.radians(direction)
        x = np.cos(direction)*length
        y = np.sin(direction)*length

        pts = np.array([pt[0]+x, pt[1]+y])

        return pts*scale

    def update2DVector(self, arrow, pt, vec, c='black', scale = 1):
    	if arrow == None:
    		arrow = self.ax1.quiver((pt*scale)[:2][0], (pt*scale)[:2][1], vec[:2][0], vec[:2][1], color=c, scale=50)
    	else:
    		arrow.set_UVC(*vec[:2])
    		arrow.set_offsets((pt*scale)[:2])
    	return arrow

    def update2DLine(self, line, pt1, pt2, c='black'):
    	if line == None:
    		line, = self.ax1.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], color=c)
    	else:
    		line.set_xdata([pt1[0], pt2[0]])
    		line.set_ydata([pt1[1], pt2[1]])
    	return line
