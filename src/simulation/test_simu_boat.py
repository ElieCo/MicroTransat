import numpy as np
import time
import matplotlib.pyplot as plt

### Useful functions

def sign(a):
	if a >= 0.0:
		return 1
	else:
		return -1

### Graphic functions

def getLinePt(pt, direction, length, scale = 1):
	x = np.cos(direction)*length
	y = np.sin(direction)*length

	pts = np.array([pt[0]+x, pt[1]+y])

	return pts*scale

def update2DVector(arrow, pt, vec, c='black', scale = 1):
	if arrow == None:
		arrow = ax1.quiver(*(pt*scale)[:2], *vec[:2], color=c, scale=50)
	else:
		arrow.set_UVC(*vec[:2])
		arrow.set_offsets((pt*scale)[:2])
	return arrow

def update2DLine(line, pt1, pt2, c='black'):
	if line == None:
		line, = ax1.plot([pt1[0], pt2[0]], [pt1[1], pt2[1]], color=c)
	else:
		line.set_xdata([pt1[0], pt2[0]])
		line.set_ydata([pt1[1], pt2[1]])
	return line

### Dynamics model functions

# a savoir : toutes les matrices sont en symétrie par rapport au normes
# enfin je crois

def updateLandmark(c, s):

	phi_0 = c["phi_0"]-s["phi_2"]/2
	phi_1 = c["phi_1"]
	phi_2 = s["phi_2"]
	alpha = s["alpha"]
	beta = s["beta"]

	# repere du bateau
	x = np.array([np.cos(alpha), np.sin(alpha), 0])
	y = np.array([-np.sin(alpha)*np.cos(beta), np.cos(alpha)*np.cos(beta), np.sin(beta)])
	z = np.array([np.sin(alpha)*np.sin(beta), -np.cos(alpha)*np.sin(beta), np.cos(beta)])
	R = np.array([x,y,z])

	# repere de la barre dans celui du bateau
	x0 = np.array([np.cos(phi_0), np.sin(phi_0), 0])
	y0 = np.array([-np.sin(phi_0), np.cos(phi_0), 0])
	z0 = np.array([0, 0, 1])
	R0 = np.array([x0,y0,z0])

	# repere de l'aileron dans celui du bateau
	x1 = np.array([-np.cos(phi_1 + phi_2), -np.sin(phi_1 + phi_2), 0])
	y1 = np.array([0, 0, 1])
	z1 = np.array([-np.sin(phi_1 + phi_2), np.cos(phi_1 + phi_2), 0])
	R1 = np.array([x1,y1,z1])

	# repere de l'aile dans celui du bateau
	x2 = np.array([-np.cos(phi_2), -np.sin(phi_2), 0])
	y2 = y1
	z2 = np.array([-np.sin(phi_2), np.cos(phi_2), 0])
	R2 = np.array([x2,y2,z2])

	s["R"] = R
	s["R0"] = R0
	s["R1"] = R1
	s["R2"] = R2

	return s

def getHelmForces(c, s):
	phi_0 = c["phi_0"]-s["phi_2"]/2

	# repere de la barre dans celui du bateau
	x0 = s["R0"][0]
	y0 = s["R0"][1]
	z0 = s["R0"][2]

	# dans (x,y,z)
	P_0 = np.array([-(0.49+np.cos(phi_0)*0.025), -np.sin(phi_0)*0.025, -0.185])

	Vb = (np.linalg.inv(s["R"].T)).dot(s["V"]) # vitesse du bateau dans (x,y,z)
	Vbrad = (np.linalg.inv(s["R"].T)).dot(s["Vrad"]) # vitesse angulaire du bateau dans (x,y,z)
	v = -Vb# + Vbrad*P_0 # vitesse de l'eau dans (x,y,z) au point P_0

	Vva0 = (np.vdot(v,x0)/np.vdot(x0,x0))*x0 + (np.vdot(v,z0)/np.vdot(z0,z0))*z0
	if np.linalg.norm(Vva0) != 0:
		theta_v0 = np.arccos(min(1,np.vdot(v,Vva0)/(np.linalg.norm(v)*np.linalg.norm(Vva0))))
	else :
		theta_v0 = np.pi/2
	theta_v0 *=  sign(np.vdot(v,y0)/np.vdot(y0,y0))
	if np.linalg.norm(v) != 0:
		F_0t = (v/np.linalg.norm(v)) * (tr0_0 + theta_v0**2 * S_0 * Ct_0) * np.vdot(v,v)
	else:
		F_0t = np.array([0,0,0])
	F_0p = (y0/np.linalg.norm(y0)) * theta_v0 * S_0 * Cp_0 * np.vdot(v,v)
	F_0 = F_0t + F_0p

	return ([[F_0, P_0]], F_0t, F_0p)

def getWingForces(c, s):

	phi_1 = c["phi_1"]
	phi_2 = s["phi_2"]

	# il faut prendre en compte le vent vitesse
	v_real = (np.linalg.inv(s["R"].T)).dot(s["Vv"]) # vitesse du vent dans (x,y,z)
	v_speed = -(np.linalg.inv(s["R"].T)).dot(s["V"]) # vitesse du bateau dans (x,y,z)
	v = v_real + v_speed

	# repere de l'aileron dans (x,y,z)
	x1 = s["R1"][0]
	y1 = s["R1"][1]
	z1 = s["R1"][2]

	# repere de l'aile dans (x,y,z)
	x2 = s["R2"][0]
	y2 = s["R2"][1]
	z2 = s["R2"][2]

	# point dans (x,y,z)
	P_1 = O + np.array([-(np.cos(phi_2)*0.54+np.cos(phi_1+phi_2)*0.03), -(np.sin(phi_2)*0.54+np.sin(phi_1+phi_2)*0.03), 0.81])
	P_2 = O + np.array([0, 0, 0.60])
	P_3 = O + np.array([np.cos(phi_2)*0.53, np.sin(phi_2)*0.53, 0.11])

	Vva1 = (np.vdot(v,x1)/np.vdot(x1,x1))*x1 + (np.vdot(v,y1)/np.vdot(y1,y1))*y1
	if np.linalg.norm(Vva1) != 0:
		theta_v1 = np.arccos(min(1,np.vdot(v,Vva1)/(np.linalg.norm(v)*np.linalg.norm(Vva1))))
	else :
		theta_v1 = np.pi/2
	theta_v1 *=  sign(np.vdot(v,z1)/np.vdot(z1,z1))
	if np.linalg.norm(v) != 0:
		F_1t = (v/np.linalg.norm(v)) * (tr0_1 + theta_v1**2 * S_1 * Ct_1) * np.vdot(v,v)
	else:
		F_1t = np.array([0,0,0])
	F_1p = (z1/np.linalg.norm(z1)) * theta_v1 * S_1 * Cp_1 * np.vdot(v,v)
	F_1m = np.array([0, 0, -m_1*g]) # dans (u,v,w)
	F_1m = (np.linalg.inv(s["R"].T)).dot(F_1m) # dans (x,y,z)
	F_1 = F_1t + F_1p + F_1m

	Vva2 = (np.vdot(v,x2)/np.vdot(x2,x2))*x2 + (np.vdot(v,y2)/np.vdot(y2,y2))*y2
	if np.linalg.norm(Vva2) != 0:
		theta_v2 = np.arccos(min(1,np.vdot(v,Vva2)/(np.linalg.norm(v)*np.linalg.norm(Vva2))))
	else:
		theta_v2 = np.pi/2
	theta_v2 *=  sign(np.vdot(v,z2)/np.vdot(z2,z2))
	if np.linalg.norm(v) != 0:
		F_2t = (v/np.linalg.norm(v)) * (tr0_2 + theta_v2**2 * S_2 * Ct_2) * np.vdot(v,v)
	else:
		F_2t = np.array([0,0,0])
	F_2p = (z2/np.linalg.norm(z2)) * theta_v2 * S_2 * Cp_2 * np.vdot(v,v)
	F_2m = np.array([0, 0, -m_2*g]) # dans (u,v,w)
	F_2m = (np.linalg.inv(s["R"].T)).dot(F_2m) # dans (x,y,z)
	F_2 = F_2t + F_2p + F_2m

	F_3 = np.array([0, 0, -m_3*g]) # dans (u,v,w)
	F_3 = (np.linalg.inv(s["R"].T)).dot(F_3) # dans (x,y,z)

	return [[F_1, P_1], [F_2, P_2], [F_3, P_3]]

def getKeelSailForces(c, s):

	# repere de la quille dans (x,y,z)
	x4 = np.array([1,0,0])
	y4 = np.array([0,1,0])
	z4 = np.array([0,0,1])

	P_4 = np.array([-0.025, 0, -0.45/2-0.07]) # dans (x,y,z)

	Vb = (np.linalg.inv(s["R"].T)).dot(s["V"]) # vitesse du bateau dans (x,y,z)
	Vbrad = (np.linalg.inv(s["R"].T)).dot(s["Vrad"]) # vitesse angulaire du bateau dans (x,y,z)
	v = -Vb# + Vbrad*P_4 # vitesse de l'eau dans (x,y,z) au point P_4

	Vva4 = (np.vdot(v,x4)/np.vdot(x4,x4))*x4 + (np.vdot(v,z4)/np.vdot(z4,z4))*z4
	if np.linalg.norm(Vva4) != 0:
		theta_v4 = np.arccos(min(1,np.vdot(v,Vva4)/(np.linalg.norm(v)*np.linalg.norm(Vva4))))
	else :
		theta_v4 = np.pi/2
	theta_v4 *=  sign(np.vdot(v,y4)/np.vdot(y4,y4))
	if np.linalg.norm(v) != 0:
		F_4t = (v/np.linalg.norm(v)) * (tr0_4 + theta_v4**2 * S_4 * Ct_4) * np.vdot(v,v)
	else:
		F_4t = np.array([0,0,0])
	F_4p = (y4/np.linalg.norm(y4)) * theta_v4 * S_4 * Cp_4 * np.vdot(v,v)
	F_4 = F_4t + F_4p

	return ([[F_4, P_4]], F_4t, F_4p)

def getKeelBulbForces(c, s):

	P_5 = np.array([-0.02, 0, -0.46])

	F_5 = np.array([0, 0, -m_5*g]) # dans (u,v,w)
	F_5 = (np.linalg.inv(s["R"].T)).dot(F_5) # dans (x,y,z)

	return [[F_5, P_5]]

def getHullForces(c, s):

	beta = s["beta"]

	# repere de la quille dans (x,y,z)
	x6 = np.array([1,0,0])
	y6 = np.array([0,np.cos(beta),np.sin(beta)])
	z6 = np.array([0,-np.sin(beta), np.cos(beta)])

	P_6 = np.array([0, 0, 0]) # dans (x,y,z)

	Vb = (np.linalg.inv(s["R"].T)).dot(s["V"]) # vitesse du bateau dans (x,y,z)
	v = -Vb # vitesse de l'eau dans (x,y,z)

	Vva6 = (np.vdot(v,x6)/np.vdot(x6,x6))*x6 + (np.vdot(v,z6)/np.vdot(z6,z6))*z6
	if np.linalg.norm(Vva6) != 0:
		theta_v6 = np.arccos(min(1,np.vdot(v,Vva6)/(np.linalg.norm(v)*np.linalg.norm(Vva6))))
	else :
		theta_v6 = np.pi/2
	theta_v6 *=  sign(np.vdot(v,y6)/np.vdot(y6,y6))
	if np.linalg.norm(v) != 0:
		F_6t = (v/np.linalg.norm(v)) * (tr0_6 + theta_v6**2 * S_6 * Ct_6) * np.vdot(v,v)
	else:
		F_6t = np.array([0,0,0])
	F_6p = (y6/np.linalg.norm(y6)) * theta_v6 * S_6 * Cp_6 * np.vdot(v,v)
	F_6 = F_6t + F_6p

	return ([[F_6, P_6]], F_6t, F_6p)

def getI(m, P):
	x = P[0]
	y = P[1]
	z = P[2]

	I11 = (y**2 + z**2)*m
	I12 = 0#-y*x*m
	I13 = 0#-z*x*m
	I21 = 0#-x*y*m
	I22 = (x**2 + z**2)*m
	I23 = 0#-z*y*m
	I31 = 0#-x*z*m
	I32 = 0#-y*z*m
	I33 = (y**2 + x**2)*m

	I = np.array([[I11, I12, I13], [I21, I22, I23], [I31, I32, I33]])

	return I

def moveWing(s, FP, dt):

	# dans (x,y,z)
	F_1 = FP[0][0]
	P_1 = FP[0][1]
	F_2 = FP[1][0]
	P_2 = FP[1][1]
	F_3 = FP[2][0]
	P_3 = FP[2][1]

	M_1 = np.cross(P_1-O, F_1)
	M_2 = np.cross(P_2-O, F_2)

	Ma = M_1 + M_2
	I_1 = getI(m_1, P_1-O) # matrice d'inertie de l'aileron dans repere du bateau
	I_2 = getI(m_2, P_2-O) # matrice d'inertie de l'aile dans repere du bateau
	I_3 = getI(m_3, P_3-O) # matrice d'inertie du poids dans repere du bateau

	I = I_1 + I_2 + I_3

	Aa = Ma.dot(np.linalg.inv(I)) # dans (x,y,z)
	Aaz = Aa[2]

	Raz = 2*s["Vaz"]
	Aaz -= Raz

	s["Vaz"] += Aaz*dt
	s["phi_2"] += s["Vaz"]*dt

	return s

def moveBoat(s, FP, dt):

	# dans (x,y,z)
	F_0 = FP[0][0]
	P_0 = FP[0][1]
	F_1 = FP[1][0]
	P_1 = FP[1][1]
	F_2 = FP[2][0]
	P_2 = FP[2][1]
	F_3 = FP[3][0]
	P_3 = FP[3][1]
	F_4 = FP[4][0]
	P_4 = FP[4][1]
	F_5 = FP[5][0]
	P_5 = FP[5][1]
	F_6 = FP[6][0]
	P_6 = FP[6][1]

	M_0 = np.cross(P_0, F_0)
	M_1 = np.cross(P_1, F_1)
	M_2 = np.cross(P_2, F_2)
	M_3 = np.cross(P_3, F_3)
	M_4 = np.cross(P_4, F_4)
	M_5 = np.cross(P_5, F_5)
	M_6 = np.cross(P_6, F_6)

	M = M_0 + M_1 + M_2 + M_3 + M_4 + M_5 + M_6

	I_0 = getI(m_0, P_0)
	I_1 = getI(m_1, P_1)
	I_2 = getI(m_2, P_2)
	I_3 = getI(m_3, P_3)
	I_4 = getI(m_4, P_4)
	I_5 = getI(m_5, P_5)
	I_P1 = getI(m_b/2, P_b)
	I_P2 = getI(m_b/2, -P_b)

	I = I_0 + I_1 + I_2 + I_3 + I_4 + I_5 + I_P1 + I_P2

	Aa = M.dot(np.linalg.inv(I)) # dans (x,y,z)
	# Aa[0] = 0 # pas de roulis

	# il faut être dans (u,v,w) pour pouvoir enlever le tangage
	Aa = s["R"].T.dot(Aa) # dans (u,v,w)
	Aa[1] = 0 # pas de tangage
	Aa = (np.linalg.inv(s["R"].T)).dot(Aa) # dans (x,y,z)

	# get previous angular speed
	Va = s["Vrad"]  # dans (u,v,w)
	Va = (np.linalg.inv(s["R"].T)).dot(Va) # dans (x,y,z)

	Ra = 3*Va # dans (x,y,z)
	Aa -= Ra
	Aa[0] = min(np.pi/2, abs(Aa[0]))*sign(Aa[0])
	Va += Aa*dt # dans (x,y,z)

	s["beta"] += Va[0]*dt # beta est bien dans (x,y,z)

	Va = s["R"].T.dot(Va) # dans (u,v,w)
	s["Vrad"] = Va

	s["alpha"] += Va[2]*dt
	s["phi_2"] -= Va[2]*dt

	F = F_0 + F_1 + F_2 + F_3 + F_4 + F_5 + F_6 # dans (x,y,z)
	Al = F/(m_0 + m_1 + m_2 + m_3 + m_4 + m_5 + m_b) # dans (x,y,z)

	Al = s["R"].T.dot(Al) # dans (u,v,w)
	Al[2] = 0 # on reste a la surface de l'eau
	Al = (np.linalg.inv(s["R"].T)).dot(Al) # dans (x,y,z)

	# get previous speed
	Vl = s["V"]  # dans (u,v,w)
	Vl = (np.linalg.inv(s["R"].T)).dot(Vl) # dans (x,y,z)

	Vl += Al*dt # dans (x,y,z)

	Vl = s["R"].T.dot(Vl) # dans (u,v,w)
	s["V"] = Vl

	s["C"][0] += Vl[0]*dt
	s["C"][1] += Vl[1]*dt

	return s

###

draw = True

if draw :
	delta1 = 1
	delta2 = 200
	fig = plt.figure(figsize=[10,10])
	ax1 = fig.add_subplot(121)
	ax1.set_ylim(bottom=-delta1, top=delta1)
	ax1.set_xlim(left=-delta1, right=delta1)
	ax2 = fig.add_subplot(122)
	ax2.set_ylim(bottom=-delta2, top=delta2)
	ax2.set_xlim(left=-delta2, right=delta2)
	fig.show()

	d_F_0 = None
	d_F_0p = None
	d_F_0t = None
	d_F_1 = None
	d_F_2 = None
	d_F_4 = None
	d_F_4p = None
	d_F_4t = None
	d_F_6 = None
	d_F_6p = None
	d_F_6t = None
	d_F = None
	d_V = None
	d_Wind = None

	d_aile = None
	d_aileron = None

	d_boat0 = None
	d_boat1 = None
	d_boat2 = None

	d_helm = None

	d_keel = None

	pt_prev = [0.0,0.0,0.0]
	pt_actual = [0.0,0.0,0.0]


# data
if True:
	wind_speed = 10 #nds
	wind_speed = 0.514444 * wind_speed # m.s-1

	nu_air = 0.018
	nu_water = 1
	factor_air = 50
	factor_water = 80

	tr0_0 = 0.001
	S_0 = 0.0198
	Ct_0 = nu_water*factor_water
	Cp_0 = nu_water*factor_water

	tr0_1 = 0.001
	S_1 = 0.072
	Ct_1 = 2*nu_air*factor_air
	Cp_1 = nu_air*factor_air

	tr0_2 = 0.001
	S_2 = 0.39525
	Ct_2 = 2*nu_air*factor_air
	Cp_2 = nu_air*factor_air

	tr0_4 = 0.001
	S_4 = 0.05625
	Ct_4 = nu_water*factor_water
	Cp_4 = nu_water*factor_water

	tr0_6 = 0.33
	S_6 = 0.0585
	Ct_6 = nu_water*factor_water
	Cp_6 = nu_water*factor_water

	g = 9.81
	m_0 = 0.06
	m_1 = 0.194
	m_2 = 0.904
	m_3 = 0.280
	m_4 = 0.0
	m_5 = 3.710
	m_b = 1.65
	P_b = np.array([1.30/2, 0, 0])
	O = np.array([0.06, 0, 0.16])

# cmd

_phi_0 = np.radians(90/2) # angle de la barre dans (x,y,z)
_phi_1 = np.radians(10*sign(_phi_0)) # angle de l'aileron dans (x2,y2,z2)

# state

_phi_2 = np.radians(0) # angle de l'aile dans (x,y,z)
_Vaz = 0 # vitesse de rotation de l'aile autour de l'ax1e z dans (x,y,z)
_Vv = np.array([0.0, 0.0, 0.0]) # vitesse du vent dans (u,v,w)
_V = np.array([0.0, 0.0, 0.0]) # vitesse du bateau dans (u,v,w)
_Vrad = np.array([00.0, 0.0, 0.0]) # vitesse angulaire du bateau dans (u,v,w)
_C = np.array([0.0, 0.0, 0.0]) # position du bateau dans (u,v,w)
_alpha = np.radians(0) # cap dans (u,v,w)
_beta = np.radians(0) # roulis dans (u,v,w)
_R = np.array([[1,0,0],[0,1,0],[0,0,1]])  # repere du bateau
_R0 = np.array([[1,0,0],[0,1,0],[0,0,1]]) # repere de la barre dans celui du bateau
_R1 = np.array([[1,0,0],[0,1,0],[0,0,1]]) # repere de l'aileron dans celui du bateau
_R2 = np.array([[1,0,0],[0,1,0],[0,0,1]]) # repere de l'aile dans celui du bateau

c = {"phi_0":_phi_0, "phi_1":_phi_1}
s = {"phi_2":_phi_2, "Vaz":_Vaz, "Vv":_Vv, "V":_V, "Vrad":_Vrad, "C":_C, "alpha":_alpha, "beta":_beta, "R":_R, "R0":_R0, "R1":_R1, "R2":_R2}

# init
dt = 0.1

i = 0
t_0 = time.time()
t_1 = time.time()
sens_0 = 1
sens_1 = 1
while i>=0:

	wind_step = 1
	if s["Vv"][1]+wind_step < -wind_speed:
		s["Vv"][1] += wind_step
	elif s["Vv"][1]-wind_step > -wind_speed:
		s["Vv"][1] -= wind_step

	i+=1

	t_time = time.time()

	s = updateLandmark(c, s)

	(FP_helm,F_0t,F_0p) = getHelmForces(c,s)
	FP_wing = getWingForces(c, s)
	(FP_keelSail,F_4t,F_4p) = getKeelSailForces(c,s)
	FP_keelBulb = getKeelBulbForces(c,s)
	(FP_hull,F_6t,F_6p) = getHullForces(c,s)

	FP = np.concatenate((FP_helm, FP_wing, FP_keelSail, FP_keelBulb, FP_hull))
	SF = np.array([0.0,0.0,0.0])
	for fp in FP:
		SF += fp[0]

	if draw:
		aile_pts_1 = getLinePt(s["R"].T.dot(FP_wing[1][1]), np.pi+s["phi_2"]+s["alpha"], 0.33-0.09)
		aile_pts_2 = getLinePt(s["R"].T.dot(FP_wing[1][1]), s["phi_2"]+s["alpha"], 0.09)
		d_aile = update2DLine(d_aile, aile_pts_1, aile_pts_2)

		aileron_pts_1 = getLinePt(s["R"].T.dot(FP_wing[0][1]), np.pi+c["phi_1"]+s["phi_2"]+s["alpha"], 0.18-0.05)
		aileron_pts_2 = getLinePt(s["R"].T.dot(FP_wing[0][1]), c["phi_1"]+s["phi_2"]+s["alpha"], 0.05)
		d_aileron = update2DLine(d_aileron, aileron_pts_1, aileron_pts_2)

		boat_pt0 = getLinePt(np.array([0,0]), s["alpha"], 0.49)
		boat_pt1 = getLinePt(np.array([0,0]), s["alpha"]+np.radians(-170), 0.49)
		boat_pt2 = getLinePt(np.array([0,0]), s["alpha"]+np.radians(170), 0.49)
		d_boat0 = update2DLine(d_boat0, boat_pt1, boat_pt2)
		d_boat1 = update2DLine(d_boat1, boat_pt1, boat_pt0)
		d_boat2 = update2DLine(d_boat2, boat_pt0, boat_pt2)

		helm_pt0 = getLinePt(s["R"].T.dot(FP_helm[0][1]), s["alpha"]+c["phi_0"]-s["phi_2"]/2, 0.02)
		helm_pt1 = getLinePt(s["R"].T.dot(FP_helm[0][1]), np.pi+s["alpha"]+c["phi_0"]-s["phi_2"]/2, 0.09-0.02)
		d_helm = update2DLine(d_helm, helm_pt0, helm_pt1)

		vec_factor = 10

		d_F_0 = update2DVector(d_F_0, s["R"].T.dot(FP_helm[0][1]), s["R"].T.dot(FP_helm[0][0])*vec_factor, c='r')
		d_F_0t = update2DVector(d_F_0t, s["R"].T.dot(FP_helm[0][1]), s["R"].T.dot(F_0t)*vec_factor, c='b')
		d_F_0p = update2DVector(d_F_0p, s["R"].T.dot(FP_helm[0][1]), s["R"].T.dot(F_0p)*vec_factor, c='g')
		d_F_1 = update2DVector(d_F_1, s["R"].T.dot(FP_wing[0][1]), s["R"].T.dot(FP_wing[0][0])*vec_factor, c='r')
		d_F_2 = update2DVector(d_F_2, s["R"].T.dot(FP_wing[1][1]), s["R"].T.dot(FP_wing[1][0])*vec_factor, c='r')
		d_F_4 = update2DVector(d_F_4, s["R"].T.dot(FP_keelSail[0][1]), s["R"].T.dot(FP_keelSail[0][0]+FP_keelBulb[0][0])*vec_factor, c='r')
		d_F_4t = update2DVector(d_F_4t, s["R"].T.dot(FP_keelSail[0][1]), s["R"].T.dot(F_4t)*vec_factor, c='b')
		d_F_4p = update2DVector(d_F_4p, s["R"].T.dot(FP_keelSail[0][1]), s["R"].T.dot(F_4p)*vec_factor, c='g')
		d_F_6 = update2DVector(d_F_6, s["R"].T.dot(FP_hull[0][1]), s["R"].T.dot(FP_hull[0][0])*vec_factor, c='orange')
		d_F_6t = update2DVector(d_F_6t, s["R"].T.dot(FP_hull[0][1]), s["R"].T.dot(F_6t)*vec_factor, c='violet')
		d_F_6p = update2DVector(d_F_6p, s["R"].T.dot(FP_hull[0][1]), s["R"].T.dot(F_6p)*vec_factor, c='brown')
		d_F = update2DVector(d_F, [0,0,0], s["R"].T.dot(SF)*vec_factor, c='black')
		d_V = update2DVector(d_V, [0,0,0], s["V"]*vec_factor, c='pink')

		d_Wind = update2DVector(d_Wind, [0.75, 0.75], s["Vv"]-s["V"], c='g')

		# print("Speed: ", np.linalg.norm(s["V"]))

		# pt = s["R"].T.dot(FP_keelSail[0][1])[1:]
		# pt = [-pt[0], pt[1]]
		# roll_keel_pt0 = getLinePt(pt, -s["beta"]+np.pi/2, 0.45/2)
		# roll_keel_pt1 = getLinePt(pt, np.pi-s["beta"]+np.pi/2, 0.45/2)
		# d_keel = update2DLine(d_keel, roll_keel_pt0, roll_keel_pt1)

		pt_actual = s["C"]
		print(pt_actual,pt_prev)
		ax2.plot([pt_prev[0], pt_actual[0]], [pt_prev[1], pt_actual[1]], color='r')
		pt_prev = []
		for p in pt_actual:
			pt_prev.append(p)

		fig.canvas.draw()
		fig.canvas.flush_events()

	s = moveWing(s, FP_wing, dt*2)
	s = moveBoat(s, FP, dt*2)

	if time.time() - t_0 > 5 or True:
		t_0 = time.time()
		step = np.radians(10)
		mi = np.radians(5)
		ma = np.radians(45)
		if c["phi_0"] > ma:
			sens_0 = -1
		elif c["phi_0"] < mi:
			sens_0 = 1
		c["phi_0"] += sens_0*dt*np.pi/80

	if time.time() - t_1 > 10 and False:
		t_1 = time.time()
		step = np.radians(25)
		limit = np.radians(90)
		if c["phi_1"]+sens_1*step > limit:
			sens_1 = -1
		elif c["phi_1"]+sens_1*step < -limit:
			sens_1 = 1
		c["phi_1"] += sens_1*step

	# exit()
	time.sleep(max(0, dt - (time.time()-t_time)))
