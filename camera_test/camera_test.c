/********************************************************************
* 		copyright (C) 2015-2019 MYIR Tech Limited. All rights reserved
*			 @file: cameratest.c
* 		  @Created: 2015-3-4
* 	  	   @Author: Kevin Su(kevin.su@myirtech.com)
* 	  @Description: test the uart function of sending and receiving 
*	  @Modify Date: 2015-3-4
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/fb.h>
//#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <bmpfile.h>

static char  *CAMERA_FILE = "/dev/video0";

/* Set from Makefile */
//#define SHOW_FPS	1
//#define LED_FLASH	1
#define SAVE_BMP	1

#define V4L_BUFFERS_DEFAULT     4
#define V4L_BUFFERS_MAX		32
#define DST_DIR		"/dev/shm/"

/* LED */
#ifdef LED_FLASH
	#define	LED_PIN	(32 + 21)
#endif

//#define DEFAULT_COUNT	(-1) /* loop forever */
#define DEFAULT_COUNT	(23) /* capture one frame */

struct buffer {
	void * start;
	size_t length;
};

static int fd_video = -1, n_buffers = V4L_BUFFERS_DEFAULT;
static struct buffer mem_buf[V4L_BUFFERS_MAX];
static unsigned char *tmpBuffer = NULL, *dispBuffer = NULL;
static unsigned int video_width = 0;
static unsigned int video_height = 0;
#ifdef SAVE_BMP
static bmpfile_t *gbmp;
#endif
#ifdef LED_FLASH
static int fd_led = -1;
#endif

void cleanup(int sig);

static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	cleanup(SIGTERM);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static inline void yuv_to_rgb16(unsigned char y,
                                unsigned char u,
                                unsigned char v,
                                unsigned char *rgb)
{
    register int r,g,b;
    int rgb16;

    r = ((1192 * (y - 16) + 1634 * (v - 128) ) >> 10) ; 
    g = ((1192 * (y - 16) - 833 * (v - 128) - 400 * (u -128) ) >> 10);
    b = ((1192 * (y - 16) + 2066 * (u - 128) ) >> 10);

    r = (r > 255 ? 255 : (r < 0 ? 0 : r));
    g = (g > 255 ? 255 : (g < 0 ? 0 : g));
    b = (b > 255 ? 255 : (b < 0 ? 0 : b));

    rgb16 = (int)(((r >> 3)<<11) | ((g >> 2) << 5)| ((b >> 3) << 0));

    *rgb = (unsigned char)(rgb16 & 0xFF);
    rgb++;
    *rgb = (unsigned char)((rgb16 & 0xFF00) >> 8);
}

void convert_to_rgb16(unsigned char *buf, unsigned char *rgb, int width, int height)
{
    int x,y,z=0;
    int blocks;

    blocks = (width * height) * 2;

    for (y = 0; y < blocks; y+=4) {
       	unsigned char Y1, Y2, U, V;

       	Y1 = buf[y + 0];
       	U = buf[y + 1];
       	Y2 = buf[y + 2];
       	V = buf[y + 3];

        yuv_to_rgb16(Y1, U, V, &rgb[y]);
        yuv_to_rgb16(Y2, U, V, &rgb[y + 2]);
    }
}

static int video_open(const char *devname)
{
	struct v4l2_capability cap;
	struct v4l2_frmsizeenum frmsize;
	struct v4l2_fmtdesc fmt;
	int dev, ret;
	int idx;

	dev = open(devname, O_RDWR);
	if (dev < 0) {
		fprintf(stderr, "Error opening device %s: %d.\n", devname, errno);
		return dev;
	}

	memset(&cap, 0, sizeof cap);
	ret = xioctl(dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		fprintf(stderr, "Error opening device %s: unable to query device.\n",
			devname);
		close(dev);
		return ret;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		fprintf(stderr, "Error opening device %s: video capture not supported.\n",
			   devname);
		fprintf(stderr, "cap.capabilities: %#.8x, V4L2_CAP_VIDEO_CAPTURE: %#.8x\n", cap.capabilities, V4L2_CAP_VIDEO_CAPTURE);
		close(dev);
		return -EINVAL;
	}

	printf("Device %s opened: %s.\n", devname, cap.card);

	printf("Support image format\n");
	idx = 0;
	while (1) {
		fmt.index = idx;
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ret = xioctl(dev, VIDIOC_ENUM_FMT, &fmt);
		if (ret) {
			break;
		}
		printf("\tDescription = %s, Capture type = %d\n",
			   fmt.description, fmt.pixelformat);
		++idx;		
	}

	printf("Support frame sizes\n");
	idx = 0;
	while (1) {
		frmsize.index = idx;
		frmsize.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		frmsize.pixel_format = V4L2_PIX_FMT_YUYV;
		ret = xioctl(dev, VIDIOC_ENUM_FRAMESIZES, &frmsize);
		if (ret) {
			break;
		}
		printf("\twidth = %d, height = %d\n",
			   frmsize.discrete.width, frmsize.discrete.height);
		++idx;
	}

	return dev;
}


static int get_pixel_depth(unsigned int fmt)
{
    int depth = 0;

    switch (fmt) {
    case V4L2_PIX_FMT_NV12:
        depth = 12;
        break;
    //case V4L2_PIX_FMT_NV12T:
    //    depth = 12;
    //    break;
    case V4L2_PIX_FMT_NV21:
        depth = 12;
        break;
    case V4L2_PIX_FMT_YUV420:
        depth = 12;
        break;

    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YVYU:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_NV61:
    case V4L2_PIX_FMT_YUV422P:
        depth = 16;
        break;

    case V4L2_PIX_FMT_RGB32:
        depth = 32;
        break;
    }

    return depth;
}

static int video_set_format(int dev, unsigned int w, unsigned int h, unsigned int format)
{
	struct v4l2_format fmt;
	 struct v4l2_pix_format pixfmt;

	int ret;

	printf("video_set_format: width: %u height: %u\n", w, h);

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = w;
	fmt.fmt.pix.height = h;
	fmt.fmt.pix.pixelformat = format;
	if (format == V4L2_PIX_FMT_JPEG) fmt.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
	fmt.fmt.pix.sizeimage = (w * h * get_pixel_depth(format)) / 8; 
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	ret = xioctl(dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		fprintf(stderr, "Unable to set format: %d.\n", errno);
		return ret;
	}

	printf("Video format set: width: %u height: %u buffer size: %u\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);
	return 0;
}

static int video_reqbufs(int dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	int ret;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = xioctl(dev, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		fprintf(stderr, "Unable to allocate buffers: %d.\n", errno);
		return ret;
	}

	printf("%u buffers allocated.\n", rb.count);
	return rb.count;
}

static int video_enable(int dev, int enable)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = xioctl(dev, enable ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		fprintf(stderr, "Unable to %s capture: %d.\n",
			enable ? "start" : "stop", errno);
		return ret;
	}

	return 0;
}

static int video_set_input(int dev, unsigned int in)
{
	int ret;

	struct v4l2_input input;

   	input.index = in;

	ret = xioctl(dev, VIDIOC_S_INPUT, &input);
	if (ret < 0)
		fprintf(stderr, "Unable to select input %u: %s.\n", input,
			strerror(errno));

	return ret;
}

static void stop_capturing(void)
{
	if(fd_video > 0)
	{
		video_enable(fd_video, 0);
	}
}

static void uninit_device(void)
{
	unsigned int i;
	/* Un-map user space buffers */
	for (i = 0; i < n_buffers; ++i)
	{
		if (mem_buf[i].start != 0 && -1 == munmap (mem_buf[i].start, mem_buf[i].length))
			printf("munmap buf %d failed!\n", i);
	}
	memset(mem_buf, 0, sizeof(mem_buf));
	
	/* Release buffers inside the driver */
	if (fd_video > 0)
		video_reqbufs(fd_video, 0);
	
	if(tmpBuffer)
	{
		free(tmpBuffer);
		tmpBuffer = NULL;
	}
	if(dispBuffer)
	{
		free(dispBuffer);
		dispBuffer = NULL;
	}
}

static void close_device(void)
{
	if (fd_video > 0)
	{
		close(fd_video);
		fd_video = -1;
	}
	
#ifdef LED_FLASH
	if (fd_led > 0) {
		close(fd_led);
		fd_led = -1;
	}
#endif
}

void cleanup(int sig)
{
	printf("%s() invoked! sig = %d\n", __func__, sig);
	if(sig == SIGINT || sig == SIGTERM) {
		stop_capturing ();
		uninit_device ();
		close_device ();
#ifdef SAVE_BMP
		if (gbmp) {
			bmp_destroy(gbmp);
			gbmp = NULL;
		}
#endif
	}
}

int framesize_select(int *width, int *height, int force)
{
	struct v4l2_frmsizeenum frmsizes;
	int framesize[10][2] = { 0 };
	int idx = 0;

	while (1) {
		frmsizes.index        = idx;
		frmsizes.type         = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		frmsizes.pixel_format = V4L2_PIX_FMT_YUYV;

		if (xioctl(fd_video, VIDIOC_ENUM_FRAMESIZES, &frmsizes)) {
			break;
		}

		if (force) {
			if ( *width == frmsizes.discrete.width &&
				*height == frmsizes.discrete.height) {
				return 0;
			}
		}
		
		framesize[idx][0] = frmsizes.discrete.width;
		framesize[idx][1] = frmsizes.discrete.height;
		++idx;
	}
	
	if (idx == 0){
		fprintf(stderr, "No framesize is supported by this video device!\n");
		return -1;
	}

	if (force) {
		printf("unsupport framesize for %dx%d, use %dx%d instead!\n",
			*width, *height, framesize[0][0], framesize[0][1]);
	}
	
	*width = framesize[0][0];
	*height = framesize[0][1];

	return force?1:0;
}

static inline void yuv_to_rgb(	unsigned char y,
								unsigned char u,
                                unsigned char v,
                                rgb_pixel_t *pixel)
{
    register int r,g,b;

    r = ((1192 * (y - 16) + 1634 * (v - 128) ) >> 10) ; 
    g = ((1192 * (y - 16) - 833 * (v - 128) - 400 * (u -128) ) >> 10);
    b = ((1192 * (y - 16) + 2066 * (u - 128) ) >> 10);

    r = (r > 255 ? 255 : (r < 0 ? 0 : r));
    g = (g > 255 ? 255 : (g < 0 ? 0 : g));
    b = (b > 255 ? 255 : (b < 0 ? 0 : b));

    pixel->red = r;
	pixel->green = g;
	pixel->blue = b;
}

#ifdef SAVE_BMP
static void save_bmp_yuv(bmpfile_t *bmp, unsigned char *buf, int width, int height)
{
	int i, x, y, blocks, tmp;
	rgb_pixel_t pixel = {0, 0, 0, 0};
	
    blocks = (width * height) * 2;
    for (x = 0, y = 0, i = 0; i < blocks; i += 4) {
       	unsigned char Y1, Y2, U, V;

       	Y1 = buf[i + 0];
       	U = buf[i + 1];
       	Y2 = buf[i + 2];
       	V = buf[i + 3];

        yuv_to_rgb(Y1, U, V, &pixel);
		bmp_set_pixel(bmp, x, y, pixel);
		x ++;
		if (x >= width) {
			x = 0;
			y ++;
		}

        yuv_to_rgb(Y2, U, V, &pixel);
		bmp_set_pixel(bmp, x, y, pixel);
		x ++;
		if (x >= width) {
			x = 0;
			y ++;
		}
    }
//	printf("set pixel finished.\n");
}
#endif

#ifdef LED_FLASH
static inline char *get_gpio_name(int pin)
{
	static char *pio_base[4] = {
		"pioA",
		"pioB",
		"pioC",
		"pioD",
		};
	static char name[32] = {0}; 
	
	if (!name[0]) {
		sprintf(name, "%s%d", pio_base[pin/32], pin%32);
	}
	
	return name;
}
static void led_init(void)
{
	char cmd[128] = {0};
	
	/* export */
	sprintf(cmd, "echo %d|tee /sys/class/gpio/export > /dev/null 2>&1", LED_PIN);
	system(cmd);
	/* set output */
	sprintf(cmd, "echo out|tee /sys/class/gpio/%s/direction > /dev/null 2>&1", get_gpio_name(LED_PIN));
	system(cmd);
	/* set default value(OFF) */
	sprintf(cmd, "echo 1|tee /sys/class/gpio/%s/value > /dev/null 2>&1", get_gpio_name(LED_PIN));
	system(cmd);
	
	if (fd_led < 0) {
		sprintf(cmd, "/sys/class/gpio/%s/value", get_gpio_name(LED_PIN));
		fd_led = open(cmd, O_RDWR);
		if (fd_led < 0) {
			fprintf(stderr, "open %s failed!\n", cmd);
		}
	}
}
static void led_flash(void)
{
	static char value = '0';
	
	if (fd_led > 0) {
		write(fd_led, &value, sizeof(value));
		value = value=='0'?'1':'0';
	}
}
#endif

static void process_image(const void *p, int size)
{
#ifdef SAVE_BMP
	static int file_cnt = 0;
	char file_name[32] = {0};
#endif
#ifdef SHOW_FPS
	struct timeval tv;
	static unsigned long last_sec = 0, frames = 0;
#endif

#ifdef SAVE_BMP
	save_bmp_yuv(gbmp,
		(unsigned char *)p,
		video_width,
		video_height);
#else
	/* dummy copy */
	memcpy(tmpBuffer, p,  size);
#endif

#if defined(SHOW_FPS) || defined(SAVE_BMP)
	frames ++;
	gettimeofday(&tv, NULL);
	if ( last_sec == 0) {
		last_sec = tv.tv_sec;
	} else if(tv.tv_sec > last_sec) {
#ifdef SAVE_BMP
		sprintf(file_name, DST_DIR"bmp_%03d.bmp", file_cnt++);
		bmp_save(gbmp, file_name);
		if (file_cnt > 10) file_cnt = 0;
		printf("fps: %2d, save to: %s\n", frames, file_name);
#endif

#if defined(SHOW_FPS) && !defined(SAVE_BMP)
		printf("fps: %2d\r\n", frames);
#endif
		last_sec = tv.tv_sec;
		frames = 0;
	}
#endif
}

static int read_frame(void)
{
	struct v4l2_buffer buf;
	unsigned int i;

	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd_video, VIDIOC_DQBUF, &buf)) {
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

	process_image(mem_buf[buf.index].start, buf.bytesused);

	if (-1 == xioctl(fd_video, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	return 1;
}

static void mainloop(int frame_count)
{
	unsigned int count;

	count = frame_count;

	while ((frame_count<0)?1:(count-- > 0)) {
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd_video, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd_video + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;
				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame())
					break;
			/* EAGAIN - continue select loop. */
		}
		
#ifdef LED_FLASH
		led_flash();
#endif
	}
}

int camera_test(int argc, char *argv[])
{
	int dev, ret, coef, count = DEFAULT_COUNT;
	unsigned int i;

	/* V4L */
	unsigned int pixelformat = V4L2_PIX_FMT_YUYV;
    struct v4l2_buffer buf;

	//signal(SIGINT, cleanup);
	//signal(SIGTERM, cleanup);

	if (argc > 1) {
		CAMERA_FILE = argv[1];
	}

	/* Open the video device. */
	fd_video = video_open(CAMERA_FILE);
	if (fd_video < 0)
		return -1;

	if (argc == 3) {
		video_width  = atoi(argv[1]);
		video_height = atoi(argv[2]);
		if (framesize_select(&video_width, &video_height, 1) < 0) {
			fprintf(stderr, "Unsupport this framesize: x-->%d, y-->%d\n",
				   video_width, video_height);
			close_device();
			return -1;
		}
	} else if(framesize_select(&video_width, &video_height, 0) < 0) {
		fprintf(stderr, "Haven't fit framesize\n");
		close_device();
		return -1;
	}
	
	video_set_input(fd_video, 0);

	if (video_set_format(fd_video, video_width, video_height, pixelformat) < 0) {
		close(fd_video);
		fd_video = -1;
		return -1;
	}

	/* Allocate buffers. */
	if ((int)(n_buffers = video_reqbufs(fd_video, n_buffers)) < 0) {
		close(fd_video);
		fd_video = -1;
		return -1;
	}
	
	/* Map the buffers. */
	for (i = 0; i < n_buffers; ++i) {
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = xioctl(fd_video, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to query buffer %u (%d).\n", i, errno);
			close(fd_video);
			fd_video = -1;
			return ret;
		}
		printf("length: %u offset: %u\n", buf.length, buf.m.offset);

		mem_buf[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_video, buf.m.offset);
		if (mem_buf[i].start == MAP_FAILED) {
			fprintf(stderr, "Unable to map buffer %u (%d)\n", i, errno);
			close(fd_video);
			fd_video = -1;
			return -1;
		}
		mem_buf[i].length = buf.length;
		
		printf("Buffer %u mapped at address %p.\n", i, mem_buf[i].start);
	}

	/* Queue the buffers. */
	for (i = 0; i < n_buffers; ++i) {
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = xioctl(fd_video, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to queue buffer (%d).\n", errno);
			close(fd_video);
			fd_video = -1;
			return ret;
		}
	}

	tmpBuffer = malloc(video_width * video_height * 2);
	dispBuffer = malloc(video_width * video_height * 4);

#ifdef SAVE_BMP
	if ((gbmp = bmp_create(video_width, video_height, 32/*depth*/)) == NULL) {
		fprintf(stderr, "Invalid depth value: %d.\n", 32/*depth*/);
		return -1;
	} else {
		printf("bmp_create success. width:%d, height:%d, depth:%d\n",
			video_width, video_height, 32/*depth*/);
	}
	//count = 10;
#endif
	
#ifdef LED_FLASH
	led_init();
#endif

	/* Start streaming. */
	video_enable(fd_video, 1);
	usleep(200*1000);
	
	mainloop(69/*count*/);

	cleanup(SIGTERM);
	
	return 0;
}

