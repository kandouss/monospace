#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "msp_config.h"
#include "msp_buffer.h"
#include "msp_input_v4l2.h"

static int              force_format;

static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

static int read_frame(msp_input_v4l2_watcher *watcher, msp_buffer *output_buffer)
{
        struct v4l2_buffer buf;
        unsigned int i;

	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(watcher->fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
			case EAGAIN:
					return 0;
			case EIO:
					/* Could ignore EIO, see spec. */
					/* fall through */
			default:
					errno_exit("VIDIOC_DQBUF");
		}
	}

	assert(buf.index < watcher->n_buffers);

	output_buffer[0].start = watcher->buffers[buf.index].start;
	output_buffer[0].length = buf.bytesused;
	
	if (-1 == xioctl(watcher->fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
        return 1;
}


msp_buffer msp_input_v4l2_get_frame(msp_input_v4l2_watcher *watcher, int timeout_s)
{
	msp_buffer output_buffer;
	for(;;){
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(watcher->fd, &fds);	
		
		tv.tv_sec = timeout_s;
		tv.tv_usec = 0;

		r = select(watcher->fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
	//		if (EINTR == errno)
	//			return buffer {-1,-1};

			errno_exit("select");
		}

		if (0 == r) {
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}
		if(read_frame(watcher,&output_buffer))
			break;
	}
	return output_buffer;
}



static void stop_capturing(msp_input_v4l2_watcher *watcher)
{
        enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(watcher->fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(msp_input_v4l2_watcher *watcher)
{
        unsigned int i;
        enum v4l2_buf_type type;

	for (i = 0; i < watcher->n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(watcher->fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(watcher->fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(msp_input_v4l2_watcher *watcher)
{
        unsigned int i;
	for (i = 0; i < watcher->n_buffers; ++i)
			if (-1 == munmap(watcher->buffers[i].start, watcher->buffers[i].length))
					errno_exit("munmap");
	free(watcher->buffers);
}

static void init_mmap(msp_input_v4l2_watcher *watcher)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = NBUFFERS;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(watcher->fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", watcher->video_device_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         watcher->video_device_name);
                exit(EXIT_FAILURE);
        }

        watcher->buffers = calloc(req.count, sizeof(*(watcher->buffers)));

        if (!watcher->buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (watcher->n_buffers = 0; watcher->n_buffers < req.count; ++ watcher->n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = watcher->n_buffers;

                if (-1 == xioctl(watcher->fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                watcher->buffers[watcher->n_buffers].length = buf.length;
                watcher->buffers[watcher->n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              watcher->fd, buf.m.offset);

                if (MAP_FAILED == watcher->buffers[watcher->n_buffers].start)
                        errno_exit("mmap");
        }
}

static void init_device(msp_input_v4l2_watcher *watcher)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if (-1 == xioctl(watcher->fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\n",
                                 watcher->video_device_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\n",
                         watcher->video_device_name);
                exit(EXIT_FAILURE);
        }
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
				fprintf(stderr, "%s does not support streaming i/o\n",
						 watcher->video_device_name);
				exit(EXIT_FAILURE);
		}

        /* Select video input, video standard and tune here. */
        CLEAR(cropcap);
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(watcher->fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl(watcher->fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {
                /* Errors ignored. */
        }


        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format) {
                fmt.fmt.pix.width       = CAM_WIDTH;
                fmt.fmt.pix.height      = CAM_HEIGHT;
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

                if (-1 == xioctl(watcher->fd, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */
        } else {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == xioctl(watcher->fd, VIDIOC_G_FMT, &fmt))
                        errno_exit("VIDIOC_G_FMT");
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

		init_mmap(watcher);
    }

static void close_device(msp_input_v4l2_watcher *watcher)
{
        if (-1 == close(watcher->fd))
                errno_exit("close");

        watcher->fd = -1;
}

static void open_device(msp_input_v4l2_watcher *watcher)
{
        struct stat st;

        if (-1 == stat(watcher->video_device_name, &st)) {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                         watcher->video_device_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                fprintf(stderr, "%s is no device\n", watcher->video_device_name);
                exit(EXIT_FAILURE);
        }

        watcher->fd = open(watcher->video_device_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == watcher->fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         watcher->video_device_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}


int msp_input_v4l2_init(msp_input_v4l2_watcher* watcher)
{

	fprintf(stdout,"Opening video device with name %s\n",watcher->video_device_name);
	open_device(watcher);
	init_device(watcher);
	start_capturing(watcher);
}

int msp_input_v4l2_close(msp_input_v4l2_watcher *watcher)
{
	stop_capturing(watcher);
	uninit_device(watcher);
	close_device(watcher);
}

void msp_input_v4l2_initialize_watcher(msp_input_v4l2_watcher *watcher, char *device_name)
{
	watcher->fd = -1;
	watcher->n_buffers = 5;
	watcher->video_device_name = device_name;	
}
