/**
 * This is part of v4lcapture library.
 * H264 conversion functions.
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
#include <errno.h>
#include <stdint.h> /* 8 bpp handling */
#include <stdbool.h>

#include <sys/sysinfo.h>

/* ffmpeg */
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"

#include "struct.h"
#include "encoding.h"
#include "error.h"
#include "util.h"

/* flagged if source fps != dst fps */
static bool half_fps;

/**
 * Maps V4L2 pixel formats to FFMpeg ones.
 * 
 *< v4l2_fmt: FourCC code of v4l2 format
 *
 *> ff_fmt: macro of ffmpeg pixel format or -1
 */
static enum AVPixelFormat map_pix_fmt(uint32_t v4l2_fmt) {
  /* length of associative map */
  unsigned int length = sizeof(pix_fmt_map) / sizeof(pix_fmt_map[0]);
  unsigned int i;
  /* find corrispective pix fmt */
  for (i=0; i<length; ++i) {
    if (pix_fmt_map[i].v4l2_fmt == v4l2_fmt) {
      return pix_fmt_map[i].ff_fmt;
    }
  }
  /* if not found exit immediately and log */
  gen_critical("pixel format %d is not valid\n", v4l2_fmt);
  return pix_fmt_map[0].ff_fmt;
}

/**
 * Return framerate of H264 stream.
 * 
 *< den: framerate denominator
 *< num: framerate numerato
 */
static int set_framerate(struct fraction framerate) {
  /* calculate framerate */
  int cap_fps = framerate.den / framerate.num;
  
  /* if <= 30 do nothing, else halve it */
  if (cap_fps > 30) {
    half_fps = true;
    return cap_fps / 2;
  } else {
    half_fps = false;
    return cap_fps;
  }
}

conv_ctx *conv_ctx_init(ctx *cntx) {
  conv_ctx *conv_context = malloc(sizeof(conv_ctx));
  clear(*conv_context);
  
  /* set src stride */
  conv_context->src_stride[0] = cntx->stride;
  
  /* destination I420 = 12 b(it)pp - 3 planes: Y (w*h), U (w*h/4), V(w*h/4) */
  conv_context->area = cntx->f_width * cntx->f_height;
  conv_context->q_area = conv_context->area / 4;
  
  conv_context->packet = malloc(sizeof(AVPacket));
  
  conv_context->filename = malloc(51);
  strcpy(conv_context->filename, "udp://239.0.0.3:12345");
  
  conv_context->format = malloc(21);
  strcpy(conv_context->format, "avi");
  
  conv_context->crf = 28;
  /* retrieve framerate based on input */
  conv_context->framerate = set_framerate(cntx->framerate);
  
  conv_context->tune = malloc(12);
  strcpy(conv_context->tune, "zerolatency");
  conv_context->preset = malloc(10);
  strcpy(conv_context->preset, "ultrafast");
  
#ifdef DEBUG
  av_log_set_level(AV_LOG_DEBUG);
#else
  av_log_set_level(AV_LOG_QUIET);
#endif /* DEBUG */

  return conv_context;
}

void rgb24_init(ctx *cntx, conv_ctx *c_cntx) {
    /* get correspective pix_fmt in ffmpeg */
    c_cntx->pix_fmt = map_pix_fmt(cntx->pix_fmt);
    
    /* context initializing (read swscale.h) */
    c_cntx->rgb_sws_ctx = sws_getContext(cntx->f_width, cntx->f_height,
        c_cntx->pix_fmt, cntx->f_width, cntx->f_height, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (!c_cntx->rgb_sws_ctx) {
      cap_critical("pixel format conversion");
    }
    
    /* set stride */
    c_cntx->rgb_dst_stride[0] = 3 * cntx->f_width;
    
    /* assign destination planes */
    c_cntx->rgb_dst[0] = malloc(3 * c_cntx->area);
    if (!c_cntx->rgb_dst[0]) {
      cap_critical("malloc");
    }
}

void rgb24(ctx *cntx, conv_ctx *c_cntx) {
  c_cntx->src[0] = cntx->work.start;
  
  /* actual conversion (read swscale.h) */
  sws_scale(c_cntx->rgb_sws_ctx, (const uint8_t *const *) c_cntx->src,
      c_cntx->src_stride, 0, cntx->f_height, c_cntx->rgb_dst,
      c_cntx->rgb_dst_stride);
}

void save_ppm(ctx *cntx, conv_ctx *c_cntx) {
  char outfile_name[20]; /* filename of file to save ppm */
  FILE *outfile; /* actual file descriptor pointer */
  
  sprintf(outfile_name, "dump%.3ld.ppm", cntx->frame);
  outfile = fopen(outfile_name, "w");
  if (!outfile) {
    cap_critical("file save");
  }
  
  /* write ppm header */
  fprintf(outfile, "P6 %d %d 255\n", cntx->f_width, cntx->f_height);
  
  fwrite(c_cntx->rgb_dst[0], c_cntx->rgb_dst_stride[0] * cntx->f_height, 1,
      outfile);
  
  fclose(outfile);
}

void scaler_init(ctx *cntx, conv_ctx *c_cntx) {
  /* get correspective pix_fmt in ffmpeg */
  c_cntx->pix_fmt = map_pix_fmt(cntx->pix_fmt);
  
  /* context initializing (read swscale.h) */
  c_cntx->sws_ctx = sws_getContext(cntx->f_width, cntx->f_height,
      c_cntx->pix_fmt, cntx->f_width, cntx->f_height, AV_PIX_FMT_YUV420P,
      SWS_BICUBIC, NULL, NULL, NULL);
  if (!c_cntx->sws_ctx) {
    cap_critical("pixel format conversion");
  }
  
  /* set strides */
  c_cntx->dst_stride[0] = cntx->f_width;
  c_cntx->dst_stride[1] = cntx->f_width / 2;
  c_cntx->dst_stride[2] = cntx->f_width / 2;
  
  /* assign destination planes */
  c_cntx->dst[0] = malloc(c_cntx->area);
  if (!c_cntx->dst[0]) {
    cap_critical("malloc");
  }
  c_cntx->dst[1] = malloc(c_cntx->q_area);
  if (!c_cntx->dst[1]) {
    cap_critical("malloc");
  }
  c_cntx->dst[2] = malloc(c_cntx->q_area);
  if (!c_cntx->dst[2]) {
    cap_critical("malloc");
  }
}

void scale(ctx *cntx, conv_ctx *c_cntx) {
  c_cntx->src[0] = cntx->work.start;
  
  /* actual conversion (read swscale.h) */
  sws_scale(c_cntx->sws_ctx, (const uint8_t *const *) c_cntx->src,
      c_cntx->src_stride, 0, cntx->f_height, c_cntx->dst, c_cntx->dst_stride);
}

void mux_encoder_init(ctx *cntx, conv_ctx *c_cntx) {
  int temp;
  
  /* register all codecs and formats, set log level */
  av_register_all();
  
  if (strstr(c_cntx->filename, "udp://")) {
    avformat_network_init();
  }
  
  /* find encoder relative to H264 */
  c_cntx->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!c_cntx->codec) {
    gen_critical("libx264 not found on this system\n");
  }
  
  /* find format context by name (mpegts) */
  c_cntx->out_fmt = av_guess_format(c_cntx->format, NULL, NULL);
  if (!c_cntx->out_fmt) {
    gen_critical("could not find %s format\n", c_cntx->format);
  }
  
  /* allocate format context based on guessed format */
  avformat_alloc_output_context2(&c_cntx->fmt_ctx, c_cntx->out_fmt,
      NULL, NULL);
  if (!c_cntx->fmt_ctx) {
    gen_critical("could not allocate %s context\n", c_cntx->format);
  }
  
  /* allocate new stream and set codec defaults into stream */
  c_cntx->strm = avformat_new_stream(c_cntx->fmt_ctx, c_cntx->codec);
  if (!c_cntx->strm) {
    gen_critical("could not allocate libx264 context into stream\n");
  }
  c_cntx->strm->id = c_cntx->fmt_ctx->nb_streams - 1;
  /* shortcut for codec context */
  c_cntx->cdc_ctx = c_cntx->strm->codec;
  
  /* stream options */
  c_cntx->strm->time_base.num = 1;
  c_cntx->strm->time_base.den = c_cntx->framerate;
  
  /* codec context options */
  c_cntx->cdc_ctx->codec_id = AV_CODEC_ID_H264;
  c_cntx->cdc_ctx->width = cntx->f_width;
  c_cntx->cdc_ctx->height = cntx->f_height;
  c_cntx->cdc_ctx->time_base = c_cntx->strm->time_base;
  c_cntx->cdc_ctx->gop_size = 1;
  c_cntx->cdc_ctx->max_b_frames = 0;
  c_cntx->cdc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  
  c_cntx->cdc_ctx->flags |= CODEC_FLAG_LOOP_FILTER;
  c_cntx->cdc_ctx->coder_type = FF_CODER_TYPE_AC;
  c_cntx->cdc_ctx->me_cmp |= FF_CMP_SSE;
  c_cntx->cdc_ctx->me_method = ME_HEX;
  c_cntx->cdc_ctx->me_subpel_quality = 5;
  c_cntx->cdc_ctx->me_range = 16;
  c_cntx->cdc_ctx->keyint_min = c_cntx->framerate / 2;
  c_cntx->cdc_ctx->scenechange_threshold = 40;
  c_cntx->cdc_ctx->i_quant_factor = 0.71;
  c_cntx->cdc_ctx->b_frame_strategy = 1;
  c_cntx->cdc_ctx->qcompress = 0.6;
  c_cntx->cdc_ctx->qmin = 10;
  c_cntx->cdc_ctx->qmax = 51;
  c_cntx->cdc_ctx->max_qdiff = 4;
  c_cntx->cdc_ctx->refs = 2;
  c_cntx->cdc_ctx->prediction_method = FF_PRED_PLANE;
  c_cntx->cdc_ctx->trellis = 0;
  c_cntx->cdc_ctx->dct_algo = FF_DCT_FASTINT;
  
  c_cntx->cdc_ctx->profile = FF_PROFILE_H264_EXTENDED;
  c_cntx->cdc_ctx->level = 40;
  c_cntx->cdc_ctx->thread_count = get_nprocs();
  
  /* check if global header are required */
  if (c_cntx->out_fmt->flags & AVFMT_GLOBALHEADER) {
    c_cntx->cdc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
  }
  
  /* set private options "crf", "tune", "preset", ecc */
  char crf[16];
  snprintf(crf, 16, "%d", c_cntx->crf);
  av_opt_set(c_cntx->cdc_ctx->priv_data, "crf", crf, 0);
  av_opt_set(c_cntx->cdc_ctx->priv_data, "tune", c_cntx->tune, 0);
  av_opt_set(c_cntx->cdc_ctx->priv_data, "preset", c_cntx->preset, 0);
  av_opt_set(c_cntx->cdc_ctx->priv_data, "x264opts",
      "sliced-threads:sync-lookahead=20:rc-lookahead=20", 0);
  
  /* open codec with set parameters */
  if (-1 == avcodec_open2(c_cntx->cdc_ctx, c_cntx->codec, NULL)) {
    gen_critical("could not open codec\n");
  }
  
  /* open output file with buffered IO */
  temp = avio_open(&c_cntx->fmt_ctx->pb, c_cntx->filename, AVIO_FLAG_WRITE);
  if (-1 == temp) {
    gen_critical("could not open \"%s\"\n", c_cntx->filename);
  }
  
  /* write muxer header */
  temp = avformat_write_header(c_cntx->fmt_ctx, NULL);
  if (-1 == temp) {
    gen_critical("could not write header to \"%s\"\n", c_cntx->filename);
  }
  
  /* allocate frame */
  c_cntx->frame = av_frame_alloc();
  if (!c_cntx->frame) {
    gen_critical("could not allocate frame %d\n", cntx->frame);
  }
  
  c_cntx->frame->format = c_cntx->cdc_ctx->pix_fmt;
  c_cntx->frame->width = c_cntx->cdc_ctx->width;
  c_cntx->frame->height = c_cntx->cdc_ctx->height;
  
  /* allocate image for previously allocated frame */
  temp = av_image_alloc(c_cntx->frame->data, c_cntx->frame->linesize,
      c_cntx->frame->width, c_cntx->frame->height, c_cntx->frame->format, 32);
  if (!temp) {
    gen_critical("could not allocate image for frame %d\n", cntx->frame);
  }
  /* free unused frame data */
  av_freep(&(c_cntx->frame->data));
  
  /* allocate and init packet */
  av_init_packet(c_cntx->packet);
  
  c_cntx->packet->size = 0;
  c_cntx->packet->data = NULL;
}

/**
 * Static pts increment.
 * 
 *> pts: pts incremented every time by one.
 */
static uint64_t get_pts() {
  static uint64_t pts = 0;
  return pts++;
}

void mux_encode(ctx *cntx, conv_ctx *c_cntx) {
  /* if framerate is halved, drop even frames */
  if (half_fps && (cntx->frame % 2 == 0)) {
    return;
  }
 
  int temp, delayed;
  
  c_cntx->frame->data[0] = c_cntx->dst[0];
  c_cntx->frame->data[1] = c_cntx->dst[1];
  c_cntx->frame->data[2] = c_cntx->dst[2];
  
  c_cntx->frame->pts = get_pts();
  
  delayed = 0;
  temp = avcodec_encode_video2(c_cntx->cdc_ctx, c_cntx->packet, c_cntx->frame,
      &delayed);
  if (-1 == temp) {
    gen_log("could not encode frame %d\n", cntx->frame);
  }
  
  if (delayed) {
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(c_cntx->packet, c_cntx->cdc_ctx->time_base,
        c_cntx->strm->time_base);
    
    /* write packet */
    av_interleaved_write_frame(c_cntx->fmt_ctx, c_cntx->packet);
  }
}

void write_cached(ctx *cntx, conv_ctx *c_cntx) {
  unsigned int i = cntx->frame + 1;
  int temp, delayed;
  
  for (delayed=1; delayed; ++i) {
    temp = avcodec_encode_video2(c_cntx->cdc_ctx, c_cntx->packet, NULL,
        &delayed);
    if (-1 == temp) {
      gen_critical("could not encode frame %d\n", i+1);
    }
    
    if (delayed) {
      /* rescale output packet timestamp values from codec to stream timebase */
      av_packet_rescale_ts(c_cntx->packet, c_cntx->cdc_ctx->time_base,
          c_cntx->strm->time_base);
    
      /* write packet */
      av_interleaved_write_frame(c_cntx->fmt_ctx, c_cntx->packet);
    }
  }
  
  /* write muxer trailer */
  av_write_trailer(c_cntx->fmt_ctx);
  
  /* close output file */
  avio_close(c_cntx->fmt_ctx->pb);
}

void rgb24_uninit(ctx *cntx, conv_ctx *c_cntx) {
  sws_freeContext(c_cntx->rgb_sws_ctx);
  
  /* free uncompressed data pointers */
  av_freep(&c_cntx->rgb_dst[0]);
}

void scaler_uninit(ctx *cntx, conv_ctx *c_cntx) {
  sws_freeContext(c_cntx->sws_ctx);
  
  /* free uncompressed data pointers */
  av_freep(&c_cntx->dst[0]);
  av_freep(&c_cntx->dst[1]);
  av_freep(&c_cntx->dst[2]);
}

void mux_encoder_uninit(ctx *cntx, conv_ctx *c_cntx) {
  /* free packet */
  av_free_packet(c_cntx->packet);
  
  /* close codec context */
  avcodec_close(c_cntx->cdc_ctx);
  
  /* free codec context and frame */
  av_free(c_cntx->cdc_ctx);
  av_frame_free(&(c_cntx->frame));
}

void conv_ctx_uninit(conv_ctx *c_cntx) {  
  /* free strings */
  free(c_cntx->filename);
  free(c_cntx->format);
  free(c_cntx->tune);
  free(c_cntx->preset);
}
