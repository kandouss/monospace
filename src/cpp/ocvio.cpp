#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <math.h>

#include "ocvio.hpp" 

using namespace cv;
using namespace std;

// Contructor
FlowTracker::FlowTracker(Mat first_frame)
{
	max_pts = 30;
	loss_pts = 10;
	good = 0;

	points.assign(max_pts,Point2f(0,0));
	status.assign(max_pts,0);
	
	frame = first_frame;
	rows = first_frame.rows;
	cols = first_frame.cols;

	get_points(first_frame);
}

void FlowTracker::push_frame(Mat new_frame)
{	
	thread t1(&FlowTracker::get_points,this, new_frame);
	Mat flow_status,err_pts;
	vector<Point2f> next_points;
	vector<int> next_status;
	calcOpticalFlowPyrLK( frame, new_frame,
				points, next_points,
				flow_status,err_pts,
				Size(15,15), 1, // search size and maxlevel
				TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, 0.03));
	flow_status.col(0).copyTo(next_status);

	get_transformation(next_points,next_status, points, status);
 
	flow_status.col(0).copyTo(status);	
	
	point_mutex.lock();
	points = next_points;
	point_mutex.unlock();

	frame = new_frame.clone();

//	status.insert(status.end(),max_pts-status.size(),0);
//	points.insert(points.end(),max_pts-points.size(),Point2f(0,0));

	t1.join();
}

void FlowTracker::get_transformation(std::vector<cv::Point2f> p1, std::vector<int> status1, std::vector<cv::Point2f> p2, std::vector<int> status2)
{
	vector<Point2f> P,Q;

	// 1. find the centroids of p1 and p2.
	Point2f cent1_p(0,0);
	Point2f cent2_p(0,0);
	size_t count_1 = 0;
	for(size_t i = 0;i<p1.size() && i < p2.size();i++)
		if(status1[i] && status2[i]){
			cent1_p += p1[count_1];
			cent2_p += p2[count_1++];
		}
	Mat cent1 = Mat(cent1_p)*(1.0/count_1);
	Mat cent2 = Mat(cent2_p)*(1.0/count_1);
	// 2. calculate the 2x2 covariance matrix
	Mat H(2,2,CV_32FC1);
	size_t count_2 = 0;
	for(size_t i = 0;i<p1.size() && i < p2.size();i++)
		if(status1[i] && status2[i]){
			H = H + (Mat(p1[count_2])-cent1)*(Mat(p2[count_2++])-cent2).t();
		}

	Mat a(2,2,CV_32FC1),
		b(2,2,CV_32FC1),
		c(2,2,CV_32FC1);
	SVD::compute(H,a,b,c);
	Mat R(2,2,CV_32FC1);
	R = c*(b.t());
	float delta_phi = atan(R.at<float>(0,1)/R.at<float>(0,0));
//	cout << R << endl << R.at<float>(0,1) << endl << R.data[1] << endl << R.data[2] << endl << R.data[3] << endl;
	cout << delta_phi << endl;
//	cout << Mat(P)*Mat(Q).t();
		
}

void FlowTracker::get_points(Mat new_frame)
{
	Mat point_mask(rows, cols, CV_8UC1, 1);
	vector<Point2f> tmp_pts;
	int num_good_pts = 0;

	for(size_t i=0; i < status.size() ;i++)
	{
		if(status[i])
		{
			circle(point_mask,points[i],10,0,-1);
			num_good_pts++;
		}
	}
	
	if(max_pts - num_good_pts > loss_pts)
	{
		goodFeaturesToTrack(new_frame,tmp_pts,max_pts-num_good_pts,0.5,10.0,point_mask);

		size_t j=0;
		point_mutex.lock();
		for(size_t i=0; i<status.size() && j < tmp_pts.size();i++)
			if(! status[i])
			{
				points[i] = tmp_pts[j++];
			}
		point_mutex.unlock();
	}
}

