#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>        
#include <linux/videodev2.h>
//#include <linux/v4l2-mediabus.h>

#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "frame.yuv"

int VIDEO_WIDTH = 1280;
int VIDEO_HEIGHT = 720;
//#define VIDEO_FORMAT V4L2_MBUS_FMT_YUYV8_2X8
//int VIDEO_FORMAT = V4L2_MBUS_FMT_RGB888_1X24;
#define BUFFER_COUNT 10

typedef struct VideoBuffer {
	void   *start;
	size_t  length;
} VideoBuffer;

int main(int argc, char **argv)
{
	int i, ret;

	if(argc >= 4){
		VIDEO_WIDTH = atoi(argv[1]);
		VIDEO_HEIGHT = atoi(argv[2]);
		//VIDEO_FORMAT = atoi(argv[3]);
	}

	printf("www  pix = %dx%d\n",VIDEO_WIDTH,VIDEO_HEIGHT);
	//打开设备
	int fd;
	fd = open(CAMERA_DEVICE, O_RDWR, 0);
	if (fd < 0) {
		printf("Open %s failed\n", CAMERA_DEVICE);
		return -1;
	}

	struct v4l2_streamparm stream_parm;
	ioctl(fd, VIDIOC_G_PARM, &stream_parm); // 获取参数
	stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(fd, VIDIOC_S_PARM, &stream_parm); // 设置参数,设置视频采集
	//获取驱动信息
	struct v4l2_capability cap;
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap); // 查询 v4l2 的能力
	if (ret < 0) {
		printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
		return ret;
	}
	//Print capability infomations
	printf("Capability Informations:\n");
	printf(" driver: %s\n", cap.driver);
	printf(" card: %s\n", cap.card);
	printf(" bus_info: %s\n", cap.bus_info);
	printf(" version: %08X\n", cap.version);
	printf(" capabilities: %08X\n", cap.capabilities);

	printf("----------------------------\n");
	//设置视频格式
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = VIDEO_WIDTH;
	fmt.fmt.pix.height      = VIDEO_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_ALTERNATE;
	ret = ioctl(fd, VIDIOC_S_FMT, &fmt); // 设置视频格式
	if (ret < 0) {
		printf("VIDIOC_S_FMT failed (%d)\n", ret);
		return ret;
	}

	//获取视频格式
	ret = ioctl(fd, VIDIOC_G_FMT, &fmt); // 获取视频格式
	if (ret < 0) {
		printf("VIDIOC_G_FMT failed (%d)\n", ret);
		return ret;
	}
	//Print Stream Format
	printf("Stream Format Informations:\n");
	printf(" type: %d\n", fmt.type);
	printf(" width: %d\n", fmt.fmt.pix.width);
	printf(" height: %d\n", fmt.fmt.pix.height);
	char fmtstr[8];
	memset(fmtstr, 0, 8);
	memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
	printf(" pixelformat: %s\n", fmtstr);
	printf(" field: %d\n", fmt.fmt.pix.field);
	printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
	printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
	printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
	printf(" priv: %d\n", fmt.fmt.pix.priv);
	printf(" raw_date: %s\n", fmt.fmt.raw_data);

	//请求分配内存
	struct v4l2_requestbuffers reqbuf;

	reqbuf.count = BUFFER_COUNT;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fd , VIDIOC_REQBUFS, &reqbuf); // 请求视频接收 buffer 
	if(ret < 0) {
		printf("VIDIOC_REQBUFS failed (%d)\n", ret);
		return ret;
	}

	//获取空间
	VideoBuffer*  framebuf = calloc( reqbuf.count, sizeof(VideoBuffer) );
	struct v4l2_buffer buf;

	for (i = 0; i < reqbuf.count; i++) 
	{
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd , VIDIOC_QUERYBUF, &buf); // buffer 设置
		if(ret < 0) {
			printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
			return ret;
		}

		//mmap buffer
		framebuf[i].length = buf.length;
		framebuf[i].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset); // 将视频 buffer 映射到用户空间的 buffer 上
		if (framebuf[i].start == MAP_FAILED) {
			printf("mmap (%d) failed: %s\n", i, strerror(errno));
			return -1;
		}

		//Queue buffer
		ret = ioctl(fd , VIDIOC_QBUF, &buf); // 队列 buffer
		if (ret < 0) {
			printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
			return -1;
		}

		printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf[i].start, framebuf[i].length);
	}

	//开始录制
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, &type); // 打开流接收
	if (ret < 0) {
		printf("VIDIOC_STREAMON failed (%d)\n", ret);
		return ret;
	}

	//Get frame
	ret = ioctl(fd, VIDIOC_DQBUF, &buf); // 获取帧数据
	if (ret < 0) {
		printf("VIDIOC_DQBUF failed (%d)\n", ret);
		return ret;
	}

	//Process the frame
	FILE *fp = fopen(CAPTURE_FILE, "wb");
	if (fp < 0) {
		printf("open frame data file failed\n");
		return -1;
	}
	fwrite(framebuf[buf.index].start, 1, buf.length, fp);
	fflush(fp);
	fclose(fp);
	system("sync");
	printf("Capture one frame saved in %s\n", CAPTURE_FILE);

	//Re-queen buffer
	ret = ioctl(fd, VIDIOC_QBUF, &buf);
	if (ret < 0) {
		printf("VIDIOC_QBUF failed (%d)\n", ret);
		return ret;
	}

	close(fd);
	//Release the resource
	for (i=0; i< 4; i++) 
	{
		munmap(framebuf[i].start, framebuf[i].length);
	}

	printf("Camera test Done.\n");
	return 0;
}

