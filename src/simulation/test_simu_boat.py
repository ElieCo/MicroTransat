import numpy as np
import time
import matplotlib.pyplot as plt

def sign(a):
	if a >= 0.0:
		return 1
	else:
		return -1


def getLinePt(pt, direction, length, scale = 1):
	x = np.cos(direction)*length
	y = np.sin(direction)*length
	
	pts = np.array([pt[0]+x, pt[1]+y])
	
	return pts*scale

def update2DVector(arrow, pt, vec, c='black', scale = 1):
	if arrow == None:
		arrow = ax.quiver(*(pt*scale)[:2], *vec[:2], color=c, scale=50)
	else:
		arrow.set_UVC(*vec[:2])
		arrow.set_offsets((pt*scale)[:2])
	return arrow

def update2DLine(line, pt1, pt2, c='black'):
	if line == None:
		line, = ax.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], color=c)
	else:
		line.set_xdata([pt1[0], pt2[0]])
		line.set_ydata([pt1[1], pt2[1]])
	return line

f = open("graph.csv", "w")
	
draw = True

if draw :
	delta = 1
	fig = plt.figure(figsize=[10,10])
	ax = fig.add_subplot(111)
	ax.set_ylim(bottom=-delta, top=delta)
	ax.set_xlim(left=-delta, right=delta)
	fig.show()
	
	d_F_1p = None
	d_F_1t = None
	d_F_1 = None
	d_F_2p = None
	d_F_2t = None
	d_F_2 = None
	
	d_aile = None
	d_aileron = None


# data

wind_speed = 10 #nds
wind_speed = 0.514444 * wind_speed # m.s-1

tr0_1 = 0.01
S_1 = 0.072
Cx_1 = 1
Cz_1 = 1

tr0_2 = 0.01
S_2 = 0.39525
Cx_2 = 1
Cz_2 = 1

v = np.array([-wind_speed, 0, 0])

phi_1 = np.radians(0) # angle de l'aileron par rapport a l'aile
phi_2 = np.radians(0) # angle de l'aile

m_1 = 0.194
m_2 = 0.904
m_3 = 0.280

# init
dt = 0.2
Vaz = 0

i = 0
t_1 = time.time()
sens_1 = 1
while i>=0:
	
	i+=1
	
	t_time = time.time()

	# repere de l'aileron dans celui du bateau
	x1 = np.array([-np.cos(phi_1 + phi_2), -np.sin(phi_1 + phi_2), 0])
	y1 = np.array([0, 0, 1])
	z1 = np.array([-np.sin(phi_1 + phi_2), np.cos(phi_1 + phi_2), 0])

	# repere de l'aile dans celui du bateau
	x2 = np.array([-np.cos(phi_2), -np.sin(phi_2), 0])
	y2 = y1
	z2 = np.array([-np.sin(phi_2), np.cos(phi_2), 0])

	O = np.array([0, 0, 0])
	P_1 = np.array([-(np.cos(phi_2)*0.54+np.cos(phi_1+phi_2)*0.03), -(np.sin(phi_2)*0.54+np.sin(phi_1+phi_2)*0.03), 0.81+0.10])
	P_2 = np.array([0, 0, 0.60+0.10])
	P_3 = np.array([np.cos(phi_2)*0.53, np.sin(phi_2)*0.53, 0.11+0.10])

	R1 = (np.vdot(v,x1)/np.vdot(x1,x1))*x1 + (np.vdot(v,y1)/np.vdot(y1,y1))*y1
	if np.linalg.norm(R1) != 0:
		theta_v1 = np.arccos(np.vdot(v,R1)/(np.linalg.norm(v)*np.linalg.norm(R1)))
	else :
		theta_v1 = np.pi/2
	theta_v1 *=  sign(np.vdot(v,z1)/np.vdot(z1,z1))
	F_1t = (v/np.linalg.norm(v)) * (tr0_1 + theta_v1**2 * S_1 * Cx_1) * np.vdot(v,v)
	F_1p = (z1/np.linalg.norm(z1)) * theta_v1 * S_1 * Cz_1 * np.vdot(v,v)
	F_1 = F_1t + F_1p
	M_1 = np.cross(O-P_1, F_1)

	R2 = (np.vdot(v,x2)/np.vdot(x2,x2))*x2 + (np.vdot(v,y2)/np.vdot(y2,y2))*y2
	if np.linalg.norm(R2) != 0:
		theta_v2 = np.arccos(np.vdot(v,R2)/(np.linalg.norm(v)*np.linalg.norm(R2)))
	else:
		theta_v2 = np.pi/2
	theta_v2 *=  sign(np.vdot(v,z2)/np.vdot(z2,z2))
	F_2t = (v/np.linalg.norm(v)) * (tr0_2 + theta_v2**2 * S_2 * Cx_2) * np.vdot(v,v)
	F_2p = (z2/np.linalg.norm(z2)) * theta_v2 * S_2 * Cz_2 * np.vdot(v,v)
	F_2 = F_2t + F_2p
	M_2 = np.cross(O-P_2, F_2)

	Ma = M_1 + M_2

	I_1 = m_1*P_1 # matrice d'inertie de l'aileron dans repere du bateau
	I_2 = m_2*P_2 # matrice d'inertie de l'aile dans repere du bateau
	I_3 = m_3*P_3 # matrice d'inertie du poids dans repere du bateau

	I = I_1 + I_2 + I_3

	Aa = -Ma/I
	Aaz = Aa[2]
	
	Raz = 1*Vaz
	Aaz -= Raz
	
	if draw and i%1 == 0:
		d_F_1p = update2DVector(d_F_1p, P_1, F_1p, c='g')
		d_F_1t = update2DVector(d_F_1t, P_1, F_1t, c='r')
		d_F_1 = update2DVector(d_F_1, P_1, F_1, c='b')
		d_F_2p = update2DVector(d_F_2p, P_2, F_2p, c='g')
		d_F_2t = update2DVector(d_F_2t, P_2, F_2t, c='r')
		d_F_2 = update2DVector(d_F_2, P_2, F_2, c='b')
		
		aile_pts_1 = getLinePt(P_2, np.pi+phi_2, 0.33-0.09)
		aile_pts_2 = getLinePt(P_2, phi_2, 0.09)
		d_aile = update2DLine(d_aile, aile_pts_1, aile_pts_2)
		
		aileron_pts_1 = getLinePt(P_1, np.pi+phi_1+phi_2, 0.18-0.05)
		aileron_pts_2 = getLinePt(P_1, phi_1+phi_2, 0.05)
		d_aileron = update2DLine(d_aileron, aileron_pts_1, aileron_pts_2)
		
		fig.canvas.draw()
		fig.canvas.flush_events()
		
	if time.time() - t_1 > 5:
		t_1 = time.time()
		step = np.radians(25)
		limit = np.radians(90)
		if phi_1+sens_1*step > limit:
			sens_1 = -1
		elif phi_1+sens_1*step < -limit:
			sens_1 = 1
		phi_1 += sens_1*step
		
	Vaz += Aaz*dt
	phi_2 += Vaz*dt
	
	
	time.sleep(max(0, dt - (time.time()-t_time)))
