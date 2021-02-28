from graphics import *
from math import *
import time
import copy
import random

win = GraphWin("Wind Simulation", 1000, 1000)

arrow_len = 25

def toInt(t):
	_t=[]
	for e in t:
		_t.append(int(e))
	return _t

def getArrow(pt, direction, value):
	l = arrow_len
	x = sin(radians(direction))*l/2
	y = cos(radians(direction))*l/2
	
	pt1 = Point(pt.getX()+x, pt.getY()-y)
	pt2 = Point(pt.getX()-x, pt.getY()+y)
	line = Line(pt1, pt2)
	max_width = l
	max_val = 30
	min_val = 5
	width = int((value-min_val)*max_width/(max_val-min_val))
	line.setWidth(max(0,width))
	head = Circle(pt1, width)
	
	return [line]#, head]
	
def drawArrow(a):
	for _ in a:
		_.draw(win)
		
def undrawArrow(a):
	for _ in a:
		_.undraw()
		
def updateArrows(arrows, table, prev_table, init=False):
	
	for i in range(len(table)):
		row = table[i]
		prev_row = prev_table[i]
		for j in range(len(row)):
			if init or toInt(row[j]) != toInt(prev_row[j]):
				
				pt = Point(i*(arrow_len+5)+arrow_len, j*(arrow_len+5)+arrow_len)
				if len(row[j]) >= 4:
					arrow = getArrow(pt, row[j][0]+row[j][2], row[j][1]+row[j][3])
				else:
					arrow = getArrow(pt, row[j][0], row[j][1])
				
				arrow_index = i*len(row)+j
				if len(arrows) > arrow_index:
					undrawArrow(arrows[arrow_index])
					arrows[arrow_index] = arrow
				else:
					arrows.append(arrow)
				drawArrow(arrows[arrow_index])
			
	return arrows

def getAverage(table, x, y):
	h = len(table)
	w = len(table[0])
	
	default_result = [table[x][y][0], table[x][y][1]]
	
	if x == 0 or x == h-1 or y == 0 or y == w-1:
		return default_result
	
	value_x = 0.0
	value_y = 0.0
	val_count= 0.0
	for i in range(max(x-1,0), min(x+1, h-1)+1):
		for j in range(max(y-1,0), min(y+1, w-1)+1):
		
			d = table[i][j][0]
			v = table[i][j][1]
			if len(table[i][j]) >= 4:
				d += table[i][j][2]
				v += table[i][j][3]
			
			v_x = v * cos(radians(d))
			v_y = v * sin(radians(d))
			
			diff_angle = abs(atan2(i-x, y-j) - radians(d))
			coeff = cos(diff_angle) + 1.1
			coeff *= v
			coeff *= coeff
			
			value_x += v_x * coeff
			value_y += v_y * coeff
			val_count += coeff
			
	if val_count != 0:
		value_x = value_x / val_count
		value_y = value_y / val_count
		direction = degrees(atan2(value_y, value_x))
		value = sqrt(value_x*value_x + value_y*value_y)
		return [direction, value]
	else:
		return default_result

def getCopyTable(table):
	_table = []
	
	for row in table:
		_row = []
		for cel in row:
			_cel = []
			for part in cel:
				_cel.append(part)
			_row.append(_cel)
		_table.append(_row)
	
	return _table

def getPerturbLine(table):
	line = []
	for i in range(0,len(table[0])):
		table[0][i].extend([0, i])
		line.append(table[0][i])
	for i in range(1,len(table)-1):
		table[i][-1].extend([i, len(table)-1])
		line.append(table[i][-1])
	for i in range(-(len(table[-1])-1),1):
		table[-1][-i].extend([len(table)-1, -i])
		line.append(table[-1][-i])
	for i in range(-(len(table)-2),0):
		table[-i][0].extend([-i, 0])
		line.append(table[-i][0])
		
	_line=[]
	for l in line:
		valid = True
		x = int(10*cos(radians(l[0])))
		y = int(10*sin(radians(l[0])))
		if l[4] == 0 and y >= 0:
			valid = False
		elif l[4] == len(table)-1 and y <= 0:
			valid = False
		elif l[5] == 0 and x <= 0:
			valid = False
		elif l[5] == len(table[0])-1 and x >= 0:
			valid = False
		
		if(valid):
			_line.append(l)
	
	return _line
	
def setPerturb(p_line, p_pos, p_dir, p_val, p_width=2):
	
	for w in range(p_width+1):
		
		coeff = sqrt(p_width-w)/sqrt(p_width)
		
		p_line[(p_pos+w)%len(p_line)][2] = p_dir*coeff
		p_line[(p_pos+w)%len(p_line)][3] = p_val*coeff
		p_line[(p_pos-w)%len(p_line)][2] = p_dir*coeff
		p_line[(p_pos-w)%len(p_line)][3] = p_val*coeff
	
def updatePerturb(p_line):
	all_null=True
	
	coeff = 0.9
	for p in p_line:
		p[2] = trunc(p[2]*coeff*10)/10.0
		p[3] = trunc(p[3]*coeff*10)/10.0
		
		if p[2] > 1 or p[3] > 0.5:
			all_null = False
			
	return all_null

def setRandomPerturbation(p_line, max_p_dir, max_p_value, max_p_width=8):
	p_pos = int(random.random()*(len(perturb_line)-1))
	p_dir = int(random.gauss(0, max_p_dir/2))
	p_val = int(random.random()*max_p_value)
	p_width = int(random.random()*(max_p_width-2))+2
	
	setPerturb(p_line, p_pos, p_dir, p_val, p_width)

height = 30
width = 30

base_dir = 45
bas_val = 10


# init table
table = []
for i in range(height):
	row = []
	for j in range(width):
		if i == 0 or i == height-1 or j == 0 or j == width-1:
			row.append([base_dir, bas_val, 0, 0])
		else:
			row.append([base_dir, bas_val])
	table.append(row)
	
	
# init perturbations
max_p_dir = 100
max_p_value = 20

perturb_line = getPerturbLine(table)

setRandomPerturbation(perturb_line, max_p_dir, max_p_value)


arrows = updateArrows([], table, table, init=True)

while True:
	t=time.time()
	
	prev_table = getCopyTable(table)
	
	average_table = []
	for i in range(height):
		average_row = []
		for j in range(width):
			average = getAverage(table, i, j)
			average_row.append(average)
		average_table.append(average_row)
		
	for i in range(height):
		for j in range(width):
			table[i][j][0]=average_table[i][j][0]
			table[i][j][1]=average_table[i][j][1]
			
	if (updatePerturb(perturb_line)):
		setRandomPerturbation(perturb_line, max_p_dir, max_p_value)
		setRandomPerturbation(perturb_line, max_p_dir, max_p_value)
		
	arrows = updateArrows(arrows, table, prev_table)
	
	time.sleep(max(0,0.2-(time.time()-t)))
	






