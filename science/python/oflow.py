import cv2
import numpy as np
import numpy.linalg as la
import math

def get_n_features(image,n,mask=None):
	# params for ShiTomasi corner detection
	feature_params = dict( maxCorners = n,
                       qualityLevel = 0.5,
                       minDistance = 10,
                       blockSize = 7 )
	return cv2.goodFeaturesToTrack(old_gray, mask = mask, **feature_params)

cap = cv2.VideoCapture('../data/video_1.mp4')
maxpts = 30
lostpts = 5

# Parameters for lucas kanade optical flow
lk_params = dict( winSize  = (15,15),
                  maxLevel = 2,
                  criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

# Create some random colors
color = np.random.randint(0,255,(100,3))
print color

# Take first frame and find corners in it
ret, old_frame = cap.read()
old_gray = cv2.cvtColor(old_frame, cv2.COLOR_BGR2GRAY)
p0 = get_n_features(old_gray,maxpts)

# Create a mask image for drawing purposes
mask = np.zeros_like(old_frame)


# Calculate the four points which determine the original image size
w,h = old_gray.shape
bounds = np.mat([[0,h],[0,w]]).T

count=0
trans=0
phi_cum = 0

while(1):
	ret,frame = cap.read()
	if ret==False:
		break
	frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

	# calculate optical flow
	p1, st, err = cv2.calcOpticalFlowPyrLK(old_gray, frame_gray, p0, None, **lk_params)
	st2 = ([x[0] for x in st.tolist()])
	
	for p_index,(new,old) in enumerate(zip(p1,p0)):
		a,b = new.ravel()
		c,d = old.ravel()
		if st[p_index]==1:
			a,b = new.ravel()
			c,d = old.ravel()
			mask = cv2.line(mask, (a,b),(c,d),color[p_index].tolist(), 2)
			frame = cv2.circle(frame,(a,b),5,color[p_index].tolist(),-1)
	cv2.multiply(mask,0.95,mask)
	img = cv2.add(frame,mask)
	

# Find centroids
	c0 = np.mean(p0[st==1],0)
	c1 = np.mean(p1[st==1],0)
	c0 = np.array([c0[0],c0[1],0])
	c1 = np.array([c1[0],c1[1],0])
	zp = zip(p0,p1)
	H = np.array([[0,0,0],[0,0,0],[0,0,0]])
	varx = 0
	vary = 0
	for i,x in enumerate(zip(p0,p1)):
		if st[i]==1:
			H = H + np.outer(
				np.array([p0[i][0][0],p0[i][0][1],0])-c0,
				np.array([p1[i][0][0],p1[i][0][1],0])-c1)
	U,S,V = la.svd(H)
	R = np.dot(V,U.T)
	trans = trans +(-1* np.dot(R,c0)+c1)
#	print S[0]/h**2/(S[1]/w**2)
#	print 1.0*h/w
	phi_cum = phi_cum + math.atan(R[0][1]/R[0][0])

	# Undo the average translation
	M = np.float32([[math.cos(phi_cum),-1*math.sin(phi_cum),-1*trans[0]],[math.sin(phi_cum),math.cos(phi_cum),-1*trans[1]]])
#	img = cv2.warpAffine(img,M,(img.shape[1],img.shape[0]))


	# Now update the previous frame and previous points
	old_gray = frame_gray.copy()
	p0 = p1

	# If more than lostpts points have been lost, then get lostpts more.
	if sum(st2) < maxpts:
		img2 = np.zeros((w,h,1), np.uint8)
		img2[:,:] = 1
#		cv2.rectangle(img2, b1[1], b1[0],0,thickness=-1)
		for ind,old_point in enumerate(p0):
			if st[ind]==1:
				a,b = old_point.ravel()
				img2 = cv2.circle(img2,(a,b),10,255,-1)
#		cv2.imshow('frame2',img2)
#		print len(st)
#		print len(get_n_features(frame_gray,maxpts-len(p0),img2))
		morefeats = get_n_features(frame_gray,maxpts-sum(st2),img2)
		if morefeats is not None:
			for new_pt in morefeats:
				if any([x==0 for x in st2]):
					p0[st2.index(0)] = new_pt
				else:
					p0 = np.append(p0,new_pt)
			p0= p0.reshape(-1,1,2)



	cv2.imshow('frame',img)
	count=count+1

	k = cv2.waitKey(30);
	if k == ord('a'):
		pass
	elif k == 27:
		break

cv2.destroyAllWindows()
cap.release()
