import numpy as np
import time
import cv2

def get_n_features(image,n,mask=None):
	# params for ShiTomasi corner detection
	feature_params = dict( maxCorners = n,
                       qualityLevel = 0.2,
                       minDistance = 7,
                       blockSize = 7 )
	return cv2.goodFeaturesToTrack(old_gray, mask = mask, **feature_params)

def closenesses(p):
	c = 0
	enump = enumerate(p)
	for i,x in enump:
		p_sbtrkt = [ ( (x[0][0]-y[0][0])**2 +(x[0][1]-y[0][1])**2 )**0.5 for j,y in enump if j is not i]
		print sum([1 for z in p_sbtrkt if z< 1])

cap = cv2.VideoCapture('../data/video_2.mp4')
maxpts = 30
lostpts = 1

# Parameters for lucas kanade optical flow
lk_params = dict( winSize  = (15,15),
                  maxLevel = 2,
                  criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

# Create some random colors
color = np.random.randint(0,255,(100,3))

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
p_tracker = range(0,len(p0))
total_p_count = len(p0)
while(1):
	t = time.time()
	ret,frame = cap.read()
	if ret==False:
		break
	frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	# calculate optical flow
	p1, st, err = cv2.calcOpticalFlowPyrLK(old_gray, frame_gray, p0, None, **lk_params)

	if len(p1) > len(p0):
		total_p_count = total_p_count + len(p1)-len(p0)	
	
	# delete points if they move too fast?
	for p_index,(new,old) in enumerate(zip(p1,p0)):
		a,b = new.ravel()
		c,d = old.ravel()
		if ( (a-c)**2 + (b-d)**2 )**0.5 > 10:
			st[p_index] = 0
	
	# Select good points
	good_new = p1[st==1]
	good_old = p0[st==1]

#	closenesses(p0)
	
#	point_inds = point_inds + (1 - np.cumsum(st))
	# draw the tracks
	for color_index,(new,old) in enumerate(zip(good_new,good_old)):
		a,b = new.ravel()
		c,d = old.ravel()
		mask = cv2.line(mask, (a,b),(c,d), color[color_index].tolist(), 2)
		frame = cv2.circle(frame,(a,b),5,color[color_index].tolist(),-1)
	img = cv2.add(frame,mask)

	# Calculate the mean difference between new and old points
	t_0 = np.mean(p1[st==1]-p0[st==1],0)
	if np.isnan(t_0).any():
		t_0 = np.array([0,0],dtype=np.float32)
	trans = trans - t_0 
#	print "Mean: \n",t_0
#	print "total: \n",trans
#	print "Variance: \n",np.var(p1[st==1]-p0[st==1],0)

	# Undo the average translation
#	M = np.float32([[1,0,-1*trans[0]],[0,1,-1*trans[1]]])
#	img = cv2.warpAffine(img,M,(img.shape[1],img.shape[0]))
	b1 = [tuple(i) for i in (bounds - trans).astype(int).tolist()]
#	print "Max x:",b1[1][0]
#	print "Min x:",b1[0][0]
#	print "Max y:",b1[1][1]
#	print "Min y:",b1[0][1]

	try:
		cv2.rectangle(img, b1[1], b1[0],(0,255,0))
	except OverflowError:
		print "b1:\n",b1
		print "bounds:\n",bounds
		print "t_0:\n",t_0
	cv2.imshow('frame',img)

	# Now update the previous frame and previous points
	old_gray = frame_gray.copy()
	p0 = good_new.reshape(-1,1,2)

	# If more than lostpts points have been lost, then get lostpts more.
	if len(p0) < maxpts - lostpts:
		img2 = np.zeros((w,h,1), np.uint8)
		img2[:,:] = 1
#		cv2.rectangle(img2, b1[1], b1[0],0,thickness=-1)
		for old_point in good_old:
			a,b = old_point.ravel()
			img2 = cv2.circle(img2,(a,b),10,0,-1)
#			print get_n_features(frame_gray,maxpts-len(p0),img2)
#		cv2.imshow('Mask frame',img2)
		new_pts = get_n_features(frame_gray,maxpts-len(p0),img2)
		if new_pts is not None:
			p0 = np.append(p0,new_pts).reshape(-1,1,2) if new_pts is not None else p0
			total_p_count = total_p_count + len(new_pts)

	# Print the elapsed time for the main loop body
	elapsed = time.time() - t
#	print "Frame "+str(count)+", main loop hz: "+str(round(1.0/elapsed,1))

	count=count+1

	k = cv2.waitKey(25);
	if k == ord('a'):
		pass
	elif k == 27:
		break

cv2.destroyAllWindows()
cap.release()
