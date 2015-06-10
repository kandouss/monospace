
typedef struct msp_input_v4l2_watcher {
	char *video_device_name;
	int fd;
	struct msp_buffer *buffers;
	size_t n_buffers;
	int out_buf;
	int frame_count;
} msp_input_v4l2_watcher;

int msp_input_v4l2_init(msp_input_v4l2_watcher*);
int msp_input_v4l2_close(msp_input_v4l2_watcher*);
msp_buffer msp_input_v4l2_get_frame(msp_input_v4l2_watcher*, int);
void msp_input_v4l2_initialize_watcher(msp_input_v4l2_watcher*, char*);
