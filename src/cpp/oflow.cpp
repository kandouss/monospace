#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iostream>
#include <thread>
#include <mutex>
//#include <vector>
#include "ocvio.hpp"

using namespace cv;
using namespace std;

//void goodFeaturesToTrack(InputArray image, OutputArray corners, int maxCorners, double qualityLevel, double minDistance, InputArray mask=noArray(), int blockSize=3, bool useHarrisDetector=false, double k=0.04 )

int main(int argc, char** argv)
{
	VideoCapture cap("../../data/video_4.mp4");
	if(!cap.isOpened())
		return -1;

	Mat frame, frame_gray;

	cap >> frame;
	cvtColor(frame,frame_gray,COLOR_BGR2GRAY);
	FlowTracker flowt(frame_gray);
	vector<Scalar> colors(30);
	for(size_t i = 0;i < 30; i ++) {
		colors[i] = Scalar(rand() % 255, rand() % 255, rand() % 255);
	}

	for(;;)
	{
		cap >> frame;
		cvtColor(frame,frame_gray,COLOR_BGR2GRAY);
		flowt.push_frame(frame_gray);

		for(uint i=0; i<flowt.status.size(); i++)
			if(flowt.status[i])
			{
//				line(mask,last_pts[i],next_pts[i],Scalar( 255,255,255 ),2);
				circle(frame,flowt.points[i],5,colors[i],-1);
			}

        if(waitKey(10) >= 0) continue;

		imshow("frame", frame);

	}

	return 0;
}
