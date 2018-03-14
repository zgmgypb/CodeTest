/**
 * This is part of v4lcapture library.
 * 
 * Copyright (C) 2015 DataSoft Srl
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h> /* bool type */

#include <linux/videodev2.h>

#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"

#ifndef INIT_H
#define INIT_H

/**
 * The 3 method of streaming from device V4L2 supports.
 */
enum io_method {
  READ,
  MMAP,
  USERPTR, /* not currently supported */
};

/**
 * This is the capture buffer, for convenience a length is maintained.
 */
struct _buffer {
  uint8_t *start;
  size_t length;
};

struct fraction {
  int num;
  int den;
};

/**
 * Context with all information to share across functions.
 */
typedef struct _ctx {
  /* device and processing info */
  char *dev_name; /* v4l2 device name */
  int fd; /* device file descriptor */
  enum io_method io; /* read, mmap or userptr */
  unsigned int stop; /* number of frames to capture */
  uint64_t frame; /* current frame */
  /* image info */
  uint32_t pix_fmt; /* FOURCC code of intended pixel format */
  double ratio; /* aspect ratio */
  unsigned int stride; /* bytes per pixel row */
  bool cropcap; /* device supports cropping */
  int dc_top; /* default crop top padding */
  int dc_left; /* default crop left padding */
  int dc_width; /* default crop rectangle width */
  int dc_height; /* default crop rectangle height */
  int c_top; /* crop top padding */
  int c_left; /* crop left padding */
  int c_width; /* crop rectangle width */
  int c_height; /* crop rectangle height */
  int df_width; /* capture rectangle default width */
  int df_height; /* capture rectangle default height */
  int f_width; /* capture rectangle width */
  int f_height; /* capture rectangle height */
  int fps_list[500]; /* list of possible framerates "num,den:num,den:..." */
  struct fraction d_framerate; /* capture default framerate in fps */
  struct fraction framerate; /* capture framerate in fps */
  /* buffers */
  unsigned int buffers_n; /* number of mmap buffers */
  struct _buffer *buffers; /* actual buffer/s */
  struct _buffer work; /* util: current buffer to work with */
} ctx;

/**
 * Parameters for the x264 encoder in Libavcodec.
 */
typedef struct _conv_ctx {
  /* scaler structures */
  struct SwsContext *sws_ctx; /* scaler context */
  struct SwsContext *rgb_sws_ctx; /* RGB scaler context */
  uint8_t *src[1]; /* source bit array (only 1 because packed format) */
  uint8_t *dst[3]; /* dest bit array (3 planes YUV) */
  uint8_t *rgb_dst[1]; /* dest bit array (1 plane RGB24) */
  int src_stride[1]; /* stride for source */
  int dst_stride[3]; /* stride for destination (I420) */
  int rgb_dst_stride[1]; /* stride for destination (RGB24) */
  unsigned int area; /* width * height */
  unsigned int q_area; /* area / 4 */
  enum AVPixelFormat pix_fmt; /* sourc pixel format */
  /* encoder structures */
  AVCodec *codec;
  AVCodecContext *cdc_ctx;
  AVFrame *frame;
  AVPacket *packet;
  /* muxer structures */
  AVOutputFormat *out_fmt;
  AVFormatContext *fmt_ctx;
  AVStream *strm;
  /* general options */
  char *filename; /* filename to ouput muxing */
  int crf; /* constant rate factor, d: 23 */
  int framerate; /* if input fps is > 30, halved, else not */
  char *format; /* muxing format in ffmpeg -f */
  char *tune; /* default: zerolatency */
  char *preset; /* default: ultrafast */
} conv_ctx;

#endif /* INIT_H */
