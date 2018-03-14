/**
 * This is part of v4lcapture library.
 * Frames extraction functions.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <fcntl.h> /* low-level I/O */
#include <linux/videodev2.h> /* video4linux2 library */

#include "struct.h"
#include "error.h"
#include "util.h"

ctx *init_ctx() {
  ctx *context = malloc(sizeof(ctx));
  clear(*context);
  
  context->dev_name = malloc(13);
  strcpy(context->dev_name, "/dev/video0");
  
  context->io = MMAP;
  context->fd = -1;
  context->cropcap = false;
  context->dc_top = -1;
  context->dc_left = -1;
  context->dc_width = -1;
  context->dc_height = -1;
  context->c_top = -1;
  context->c_left = -1;
  context->c_width = -1;
  context->c_height = -1;
  context->df_width = -1;
  context->df_height = -1;
  context->f_width = -1;
  context->f_height = -1;
  context->d_framerate.num = -1;
  context->d_framerate.den = -1;
  context->framerate.num = -1;
  context->framerate.den = -1;
  context->pix_fmt = v4l2_fourcc('Y', 'U', 'Y', 'V');
  context->ratio = -1.0;
  
  memset(context->fps_list, -1, sizeof(context->fps_list));
  
  /* log */
  gen_log("created context\n");
  
  return context;
}

static void get_default(ctx *cntx) {
  struct v4l2_cropcap cropcap; /* source cropping rectangle */
  clear(cropcap);
  
  /* use video capture buffer as target */
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  /* execute only if device has cropcap capability */
  if (-1 < xioctl(cntx->fd, VIDIOC_CROPCAP, &cropcap)) {
    cntx->cropcap = true;
    /* save defaults */
    cntx->dc_top = cropcap.defrect.top;
    cntx->dc_left = cropcap.defrect.left;
    cntx->dc_width = cropcap.defrect.width;
    cntx->dc_height = cropcap.defrect.height;
  } else {
    cap_log("VIDIOC_CROPCAP");
  }
  
  struct v4l2_format format; /* format of the ouput (size and colorspace) */
  clear(format);

  /* use video capture buffer as target */
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  /* get default parameters */
  if (-1 < xioctl(cntx->fd, VIDIOC_G_FMT, &format)) {
    /* save defaults */
    cntx->df_width = format.fmt.pix.width;
    cntx->df_height = format.fmt.pix.height;
    cntx->ratio = ((double) cntx->df_width) / cntx->df_height;
  } else {
    cap_critical("VIDIOC_G_FMT");
  }
  
  struct v4l2_streamparm strparm;
  clear(strparm);
  
  /* use video capture buffer as target */
  strparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if (-1 < xioctl(cntx->fd, VIDIOC_G_PARM, &strparm)) {
    cntx->d_framerate.num = strparm.parm.capture.timeperframe.numerator;
    cntx->d_framerate.den = strparm.parm.capture.timeperframe.denominator;
  } else {
    cap_log("VIDIOC_G_PARM");
  }
  
  /* log */
  gen_log("gathered default parameters\n");
}

void open_device(ctx *cntx) {
  struct stat st;

  /* catches identification errors */
  if (-1 == stat(cntx->dev_name, &st)) {
    gen_critical("cannot identify \"%s\": %d, %s\n", cntx->dev_name, errno,
        strerror(errno));
  }

  /* catches invalid device error (eg. provided file is not a device) */
  if (!S_ISCHR(st.st_mode)) {
    gen_critical("%s is not a device\n", cntx->dev_name);
  }

  /* open device free of errors using nonblocking I/O */
  /* O_NONBLOCK not supported */
  cntx->fd = open(cntx->dev_name, O_RDWR | O_NONBLOCK, 0);

  /* check if device is opened correctly */
  if (-1 == cntx->fd) {
    gen_critical("cannot open \"%s\": %d, %s\n", cntx->dev_name, errno,
        strerror(errno));
  }
  
  /* log */
  gen_log("opened device %s\n", cntx->dev_name);
  
  /* set default parameters in context */
  get_default(cntx);
}

void v4l2_crop(ctx *cntx) {
  struct v4l2_crop crop; /* target cropping rectangle */
  clear(crop);

  /* use video capture buffer as target */
  crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  /* set crop rectangle */
  if (check(cntx->c_top, cntx->dc_top)) {
    crop.c.top = cntx->c_top;
  } else {
    crop.c.top = cntx->dc_top;
  }
  if (check(cntx->c_left, cntx->dc_left)) {
    crop.c.left = cntx->c_left;
  } else {
    crop.c.left = cntx->dc_left;
  }
  if (check(cntx->c_width, cntx->dc_width)) {
    crop.c.width = cntx->c_width;
  } else {
    crop.c.width = cntx->dc_width;
  }
  if (check(cntx->c_height, cntx->dc_height)) {
    crop.c.height = cntx->c_height;
  } else {
    crop.c.height = cntx->dc_height;
  }
  
  /* applying crop with a ioctl sys call */
  if (-1 == xioctl(cntx->fd, VIDIOC_S_CROP, &crop)) {
    cntx->cropcap = false;
    if (EINVAL == errno) {
      dev_log(cntx->dev_name, "detected an error in cropping parameters");
    } else {
      cap_error("VIDIOC_S_CROP");
    }
  } else {
    /* save final crop rectangle */
    clear(crop);
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (-1 < xioctl(cntx->fd, VIDIOC_G_CROP, &crop)) {
      cntx->c_top = crop.c.top;
      cntx->c_left = crop.c.left;
      cntx->c_width = crop.c.width;
      cntx->c_height = crop.c.height;
    } else {
      cap_log("VIDIOC_G_CROP");
    }
  }
}

int v4l2_format(ctx *cntx) {
  struct v4l2_format format; /* format of the ouput (size and colorspace) */
  clear(format);

  /* use video capture buffer as target */
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  /* set size */
  if (check(cntx->f_width, cntx->df_width)) {
    format.fmt.pix.width = cntx->f_width;
  } else {
    format.fmt.pix.width = cntx->df_width;
  }
  if (check(cntx->f_height, cntx->df_height)) {
    format.fmt.pix.height = cntx->f_height;
  } else {
    format.fmt.pix.height = cntx->df_height;
  }
  
  /* chose pixel format */
  format.fmt.pix.pixelformat = cntx->pix_fmt;

  /* set non interlaced */
  format.fmt.pix.field = V4L2_FIELD_NONE;

  /* actually set parameters with a ioctl call */
  if (-1 == xioctl(cntx->fd, VIDIOC_S_FMT, &format)) {
    cap_critical("VIDIOC_S_FMT");
  }
  
  /* save final crop rectangle */
  clear(format);
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if (-1 < xioctl(cntx->fd, VIDIOC_G_FMT, &format)) {
    cntx->f_width = format.fmt.pix.width;
    cntx->f_height = format.fmt.pix.height;
    cntx->stride = format.fmt.pix.bytesperline;
  } else {
      cap_critical("VIDIOC_G_FMT");
  }
  
  return format.fmt.pix.sizeimage;
}

void v4l2_list_framerate(ctx *cntx) {
  struct v4l2_frmivalenum fps;
  clear(fps);
  
  /* set parameters */
  fps.pixel_format = cntx->pix_fmt;
  fps.width = cntx->f_width;
  fps.height = cntx->f_height;
  
  /* populate list_fps */
  if (-1 == xioctl(cntx->fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps)) {
    cap_critical("VIDIOC_ENUM_FRAMEINTERVALS");
  }
  
  /* discrete time */
  if (fps.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
    unsigned int i = 0;
    while (-1 != xioctl(cntx->fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps) &&
        i < 10) {
      cntx->fps_list[i] = fps.discrete.numerator;
      cntx->fps_list[i+1] = fps.discrete.denominator;
      fps.index += 1;
      i += 2;
    }
  }
  
  /* stepwise or continuous type */
  if (fps.type == V4L2_FRMIVAL_TYPE_STEPWISE ||
      fps.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
    unsigned int k, i = 0;
    int min_num = fps.stepwise.min.numerator;
    int min_den = fps.stepwise.min.denominator;
    int max_num = fps.stepwise.max.numerator;
    int max_den = fps.stepwise.max.denominator;
    
    /* check min and max num are equal */
    assert(min_num == max_num);
    
    for (k = min_den; k >= max_den && i < 500; --k, i+=2) {
      cntx->fps_list[i] = min_num;
      cntx->fps_list[i+1] = k;
    }
  }
  
  /* recall get_default to restore default parameters */
  get_default(cntx);
}

void v4l2_framerate(ctx *cntx) {
  struct v4l2_streamparm strparm;
  clear(strparm);
  
  /* use video capture buffer as target */
  strparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  /* set framerate */
  if (check(cntx->framerate.num, cntx->d_framerate.num)) {
    strparm.parm.capture.timeperframe.numerator = cntx->framerate.num;
  } else {
    strparm.parm.capture.timeperframe.numerator = cntx->d_framerate.num;
  }
  if (check(cntx->framerate.den, cntx->d_framerate.den)) {
    strparm.parm.capture.timeperframe.denominator = cntx->framerate.den;
  } else {
    strparm.parm.capture.timeperframe.denominator = cntx->d_framerate.den;
  }
  
  /* ioctl sys call to actually set framerate */
  if (-1 == xioctl(cntx->fd, VIDIOC_S_PARM, &strparm)) {
    cap_log("VIDIOC_S_PARM");
  }
  
  /* save final framerate */
  clear(strparm);
  strparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if (-1 < xioctl(cntx->fd, VIDIOC_G_PARM, &strparm)) {
    cntx->framerate.num = strparm.parm.capture.timeperframe.numerator;
    cntx->framerate.den = strparm.parm.capture.timeperframe.denominator;
  } else {
    cap_log("VIDIOC_G_PARM");
  }
}

/**
 * Initialize buffers for read/write method.
 *
 *< cntx: context
 *< buffer_size: number of buffers to initialize
 */
static void init_read(ctx *cntx, unsigned int buffer_size) {
  /* reserve memory for buffers and check errors */
  cntx->buffers = calloc(1, sizeof(struct _buffer));
  if (!cntx->buffers) {
    cap_critical("malloc");
  }

  /* set length and malloc and catch errors */
  cntx->buffers[0].length = buffer_size;
  cntx->buffers[0].start = malloc(buffer_size);
  if (!cntx->buffers[0].start) {
    cap_critical("malloc");
  }
}

/**
 * Request and initialize buffers for mmap method.
 *
 *< cntx: context
 */
static void init_mmap(ctx *cntx) {
  struct v4l2_requestbuffers reqbuf; /* request buffers */
  clear(reqbuf);

  /* setting up type, memory type and number of buffers */
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = 4;

  /* actually requesting buffers with a ioctl sys call */
  if (-1 == xioctl(cntx->fd, VIDIOC_REQBUFS, &reqbuf)) {
    if (EINVAL == errno) {
      dev_critical(cntx->dev_name, "does not support memory mapping method");
    } else {
      cap_critical("VIDIOC_REQBUFS");
    }
  }

  /* exit if a sufficient number of buffer (5) is not present */
  if (reqbuf.count < 4) {
      dev_log(cntx->dev_name, "has an insufficient number of buffers");
  }

  /* allocating memory for buffers and checking errors */
  cntx->buffers = calloc(reqbuf.count, sizeof(struct _buffer));
  if (!(cntx->buffers)) {
    cap_critical("malloc");
  }

  /* initializing every buffer present */
  for (cntx->buffers_n=0; cntx->buffers_n<reqbuf.count; ++cntx->buffers_n) {
    struct v4l2_buffer buffer;
    clear(buffer);

    /* setting up type, memory type and index number */
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = cntx->buffers_n;

    /* actually requesting buffers */
    if (-1 == xioctl(cntx->fd, VIDIOC_QUERYBUF, &buffer)) {
      cap_critical("VIDIOC_QUERYBUF");
    }

    /* intializing and checking error */
    cntx->buffers[cntx->buffers_n].length = buffer.length; /* saving length for freeing later */
    cntx->buffers[cntx->buffers_n].start = mmap(NULL, buffer.length,
        PROT_READ | PROT_WRITE, MAP_SHARED, cntx->fd, buffer.m.offset);

    if (MAP_FAILED == cntx->buffers[cntx->buffers_n].start) {
      cap_critical("mmap");
    }
  }
}

/**
 * Initialize request buffers for user pointer method.
 */
static void init_userptr(void) {
  exit(ENOSYS); /* ENOSYS -> function not implemented */
}

void init_device(ctx *cntx) {
  struct v4l2_capability cap; /* device capabilities */
  
  /* query device capabilities and catch errors */
  if (-1 == xioctl(cntx->fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      dev_critical(cntx->dev_name, "is not a valid V4L2 device");
    } else {
      cap_critical("VIDEOC_QUERYCAP");
    }
  }

  /* check if device is a video capture one */
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    dev_critical(cntx->dev_name, "is not a capture device");
  }

  /* check device support for read, mmap or userptr streaming method */
  /* if streaming i/o does not work, fall back to read */
  switch (cntx->io) {
    case MMAP:
    case USERPTR:
      if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        dev_log(cntx->dev_name,
            "does not support streaming I/O, falling back to read/write");
      } else {
        break;
      }
    case READ:
      if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
        dev_critical(cntx->dev_name, "does not support read/write I/O");
      }
      break;
  }

  /* set height, width, color space and interlace */
  int buffer_size = v4l2_format(cntx);
  
  /* if supported */
  if (cntx->cropcap) {
    /* crops output to view */
    v4l2_crop(cntx);
  }
  
  /* fullfill fps_list */
  v4l2_list_framerate(cntx);
  
  /* set framerate */
  v4l2_framerate(cntx);
  
  switch (cntx->io) {
    case READ:
      init_read(cntx, buffer_size);
      break;
    case MMAP:
      init_mmap(cntx);
      break;
    case USERPTR:
      /* not implemented */
      init_userptr();
      break;
  }
  
  /* log */
  gen_log("device %s is ready to capture frames\n", cntx->dev_name);
}

void start_capture(ctx *cntx) {
  enum v4l2_buf_type type; /* type of buffer */

  switch (cntx->io) {
    case READ:
      break;
    case MMAP: {
      /* allocate buffer definitively */
      unsigned int j;
      for (j=0; j<cntx->buffers_n; ++j) {
        /* create, set up and ioctl buffer */
        struct v4l2_buffer buffer;
        clear(buffer);

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = j;
        
        /* queue frame */
        if (-1 == xioctl(cntx->fd, VIDIOC_QBUF, &buffer)) {
          cap_critical("VIDIOC_QBUF");
        }
      }

      /* start capturing */
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == xioctl(cntx->fd, VIDIOC_STREAMON, &type)) {
        cap_critical("VIDIOC_STREAMON");
      }
      break;
    }
    case USERPTR:
      /* not implemented */
      break;
  }
  
  /* log */
  gen_log("started capturing\n");
}

void capture_timeout(ctx *cntx) {
  fd_set fds;
  struct timeval tv;

  /* choose device opened */
  FD_ZERO(&fds);
  FD_SET(cntx->fd, &fds);

  /* set timeout in 2 seconds */
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  /* wait up to 2 sec */
  int temp;
  temp = select(cntx->fd+1, &fds, NULL, NULL, &tv);
  switch (temp) {
    case -1:
      if (EINTR == errno) {
        gen_log("interrupted select syscall\n");
      }
      cap_critical("select");
      break;
    case 0:
      gen_critical("select timeout\n");
      break;
  }
}

int read_frame(ctx *cntx) {
  struct v4l2_buffer buffer;

  switch (cntx->io) {
    case READ:
      /* fetch frame through read sys call */
      if (-1 == read(cntx->fd, cntx->buffers[0].start, cntx->buffers[0].length)) {
        switch (errno) {
          case EAGAIN:
            /* go to next while cycle */
            return 0;
          case EIO:
            /* EIO ignored, following specs */
          default:
            cap_critical("read");
        }
      } else {
        cntx->frame++;
      }
      
      /* fill temp buffer */
      cntx->work.start = cntx->buffers[0].start;
      cntx->work.length = cntx->buffers[0].length;
      
      break;
    case MMAP:
      /* initialize memory map buffer */
      clear(buffer);
      
      buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buffer.memory = V4L2_MEMORY_MMAP;

      /* fetch frame through ioctl sys call */
      if (-1 == xioctl(cntx->fd, VIDIOC_DQBUF, &buffer)) {
        switch (errno) {
          case EAGAIN:
            /* continue the cycle */
            return 0;
          case EIO:
            /* EIO ignored, following specs */
          default:
            cap_critical("VIDIOC_DQBUF");
        }
      } else {
        cntx->frame++;
      }
      
      /* make sure a segmentation fault is not happening */
      assert(buffer.index < cntx->buffers_n);
      
      /* fill temp buffer */
      cntx->work.start = cntx->buffers[buffer.index].start;
      cntx->work.length = cntx->buffers[buffer.index].length;
      
      if (-1 == xioctl(cntx->fd, VIDIOC_QBUF, &buffer)) {
        cap_critical("VIDIOC_QBUF");
      }
      break;
    case USERPTR:
      /* not implemented */
      break;
  }

  return 1;
}

void stop_capture(ctx *cntx) {
  enum v4l2_buf_type type;

  switch (cntx->io) {
    case READ:
      break;
    case MMAP:
      /* mark device inactive */
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == xioctl(cntx->fd, VIDIOC_STREAMOFF, &type)) {
        cap_critical("VIDIOC_STREAMOFF");
      }
      break;
    case USERPTR:
      /* not implemented */
      break;
  }
  
  /* log */
  gen_log("stopped capturing\n");
}

void uninit_device(ctx *cntx) {
  switch (cntx->io) {
    case READ:
      /* free the only open buffer */
      free(cntx->buffers[0].start);
      break;
    case MMAP: {
      /* unmap all allocated memory maps */
      unsigned int j;
      for (j=0; j<cntx->buffers_n; ++j) {
        if (-1 == munmap(cntx->buffers[j].start, cntx->buffers[j].length)) {
          cap_critical("munmap");
        }
      }
      break;
    }
    case USERPTR:
      /* not implemented */
      break;
  }

  free(cntx->buffers);
}

void close_device(ctx *cntx) {
  /* close and catch error */
  if (-1 == close(cntx->fd)) {
    cap_critical("close");
  }
  cntx->fd = -1;
  
  /* log */
  gen_log("closed device %s\n", cntx->dev_name);
  
  free(cntx->dev_name);
}
