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

#include <stdint.h>

/* ffmpeg */
#include "libavutil/pixfmt.h"

#include "struct.h"

#ifndef PIXFMT_H
#define PIXFMT_H

/* bridge between V4L2 pix_fmt and ffmpeg pix_fmt */
struct pixel_format {
  enum AVPixelFormat ff_fmt;
  uint32_t v4l2_fmt;
};

static const struct pixel_format pix_fmt_map[] = {
    /* ff_fmt */         /* v4l2_fmt */
  { AV_PIX_FMT_NONE,     -1                   },
  { AV_PIX_FMT_YUV420P,  V4L2_PIX_FMT_YVU420  },
  { AV_PIX_FMT_YUV422P,  V4L2_PIX_FMT_YUV422P },
  { AV_PIX_FMT_YUYV422,  V4L2_PIX_FMT_YUYV    },
  { AV_PIX_FMT_UYVY422,  V4L2_PIX_FMT_UYVY    },
  { AV_PIX_FMT_YUV411P,  V4L2_PIX_FMT_YUV411P },
  { AV_PIX_FMT_YUV410P,  V4L2_PIX_FMT_YUV410  },
  { AV_PIX_FMT_RGB555LE, V4L2_PIX_FMT_RGB555  },
  { AV_PIX_FMT_RGB555BE, V4L2_PIX_FMT_RGB555X },
  { AV_PIX_FMT_RGB565LE, V4L2_PIX_FMT_RGB565  },
  { AV_PIX_FMT_RGB565BE, V4L2_PIX_FMT_RGB565X },
  { AV_PIX_FMT_BGR24,    V4L2_PIX_FMT_BGR24   },
  { AV_PIX_FMT_RGB24,    V4L2_PIX_FMT_RGB24   },
  { AV_PIX_FMT_BGR0,     V4L2_PIX_FMT_BGR32   },
  { AV_PIX_FMT_0RGB,     V4L2_PIX_FMT_RGB32   },
  { AV_PIX_FMT_GRAY8,    V4L2_PIX_FMT_GREY    },
};

#endif /* PIXFMT_H */

/**
 * Creates a conversion context with default values applied.
 * 
 *< cntx: pointer to general context
 * 
 *> conv_context: pointer to ctx with default values.
 */
conv_ctx *conv_ctx_init(ctx *cntx);

/**
 * Initialize and allocate structures for rgb24 scaler,
 * such as scaler context, strides and dest buffers.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void rgb24_init(ctx *cntx, conv_ctx *c_cntx);

/**
 * Converts from input pixel format to RGB24 (8 bpc).
 * 
 *< cntx: context
 *< c_cntx: pointer to conversion context
 */
void rgb24(ctx *cntx,  conv_ctx *c_cntx);

/**
 * Utility function to save temp buffer into ppm images.
 * 
 *< cntx: context
 */
void save_ppm(ctx *cntx, conv_ctx *c_cntx);

/**
 * Initialize and allocate structures for scaler,
 * such as scaler context, strides and dest buffers.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void scaler_init(ctx *cntx, conv_ctx *c_cntx);

/**
 * Converts from input pixel format to I420 (known as YUV420P).
 * 
 *< cntx: context
 *< c_cntx: pointer to conversion context
 */
void scale(ctx *cntx,  conv_ctx *c_cntx);

/**
 * Initialize and allocate structures for encoder and muxer,
 * such as codec, format, codec and format context, stream, ecc, and set
 * h264 options and muxer options.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void mux_encoder_init(ctx *cntx, conv_ctx *c_cntx);

/**
 * Does the encoding and muxing frame for frame, saving the output to a file.
 * Allocate a frame, an image and a packet for every frame captured. 
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void mux_encode(ctx *cntx, conv_ctx *c_cntx);

/**
 * Writes eventually delayed frames to file previously opened. 
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void write_cached(ctx *cntx, conv_ctx *c_cntx);

/**
 * Uninitialize rgb24 scaler structures, such as its context.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void rgb24_uninit(ctx *cntx, conv_ctx *c_cntx);

/**
 * Uninitialize scaler structures, such as its context.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void scaler_uninit(ctx *cntx, conv_ctx *c_cntx);

/**
 * Writes muxer trailer and closes file.
 * Closes codec and frees remaining malloc'd structures.
 * 
 *< cntx: pointer to general context
 *< c_cntx: pointer to conversion context
 */
void mux_encoder_uninit(ctx *cntx, conv_ctx *c_cntx);

/**
 * Free the conversion context malloc'd variables.
 * 
 *< c_cntx: pointer to conversion context
 */
void conv_ctx_uninit(conv_ctx *c_cntx);
