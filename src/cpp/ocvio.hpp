
class FlowTracker
{
	public:
	cv::Mat frame;
	std::vector<cv::Point2f> points;
	std::vector<int> status;
	std::mutex point_mutex;
	std::mutex frame_mutex;

	int check_points();
	void get_points(cv::Mat new_frame);

 //   public:
    int max_pts;
    int loss_pts;
	int rows, cols;
	int good;

    // constructor
    FlowTracker(cv::Mat first_frame);

    // Methods
    void push_frame(cv::Mat last_f);

	void get_transformation(std::vector<cv::Point2f> p1, std::vector<int> status1, std::vector<cv::Point2f> p2, std::vector<int> status2);
};
