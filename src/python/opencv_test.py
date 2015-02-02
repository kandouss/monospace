import numpy as np
import cv2

cap = cv2.VideoCapture('../data/video_1.mp4')

blur = 0.9

rolling = []
while(cap.isOpened()):
	ret,frame = cap.read()

	if ret:

		new_frame = cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
		rolling = cv2.addWeighted(rolling,blur,new_frame,1.0-blur,0.0) if rolling!=[] else new_frame
		
		cv2.imshow('gray frame',new_frame)
		cv2.imshow('rolling frame',rolling)

		if cv2.waitKey(10) & 0xFF == ord('q'):
			break
	else:
		break


cap.release()
cv2.destroyAllWindows()
