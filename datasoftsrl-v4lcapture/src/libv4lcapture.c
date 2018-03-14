/**
 * This is part of v4lcapture library.
 * Python bindings.
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

/**
 * Every method wraps one or more methods defined in the library.
 * This has been made for convenience, in order to work only with primitives,
 * with ctypes.
 * 
 * If wrapping method defines a new convention, it will be specified in its
 * comment.
 */

#include <string.h>
#include <stdint.h>

#include "struct.h"
#include "capture.h"
#include "encoding.h"
#include "util.h"
#include "error.h"

/*******************************************************************************
 * METADATA                                                                    *
 ******************************************************************************/
const char *VERSION = "1.0.1";
/*******************************************************************************
 ******************************************************************************/

ctx *cntx; /* global general context */
conv_ctx *c_cntx; /* global scaling, encoding and muxing context */

const char *version(void) {
  return VERSION;
}

void setDevice(const char *s) {
  strncpy(cntx->dev_name, s, 12);
}

/**
 * 1: READ
 * 2: MMAP
 */
void setIO(int sel) {
  switch (sel) {
    case 1:
      cntx->io = READ;
      break;
    case 2:
    default:
      cntx->io = MMAP;
      break;
  }
}

/**
 * 0: infinite
 * >0: as specified
 */
void setFrames(int64_t no) {
  if (1 > no) {
    cntx->stop = 0;
  } else {
    cntx->stop = no;
  }
}

void setPixFmt(char a, char b, char c, char d) {
  cntx->pix_fmt = v4l2_fourcc(a, b, c, d);
}

void setCrop(int top, int left, int width, int height) {
  cntx->c_top = top;
  cntx->c_left = left;
  cntx->c_width = width;
  cntx->c_height = height;
}

void setFormat(int width, int height) {
  cntx->f_width = width;
  cntx->f_height = height;
}

void setFps(int num, int den) {
  cntx->framerate.num = num;
  cntx->framerate.den = den;
}

char *getDevice(void) {
  return cntx->dev_name;
}

/**
 * 1: READ
 * 2: MMAP
 */
int getIO(void) {
  switch (cntx->io) {
    case READ:
      return 1;
    case MMAP:
    default:
      return 2;
  }
}

int64_t getFrames(void) {
  return cntx->stop;
}

int getCropcap(void) {
  return cntx->cropcap;
}

/**
 * Returns: [top, left, width, height]
 */
int *getDefCrop(void) {
  int *ret = malloc(4 * sizeof(int));
  ret[0] = cntx->dc_top;
  ret[1] = cntx->dc_left;
  ret[2] = cntx->dc_width;
  ret[3] = cntx->dc_height;
  return ret;
}

/**
 * Returns: [top, left, width, height]
 */
int *getCrop(void) {
  int *ret = malloc(4 * sizeof(int));
  ret[0] = cntx->c_top;
  ret[1] = cntx->c_left;
  ret[2] = cntx->c_width;
  ret[3] = cntx->c_height;
  return ret;
}

/**
 * Returns: [width, height]
 */
int *getDefFormat(void) {
  int *ret = malloc(2 * sizeof(int));
  ret[0] = cntx->df_width;
  ret[1] = cntx->df_height;
  return ret;
}

/**
 * Returns: [width, height]
 */
int *getFormat(void) {
  int *ret = malloc(2 * sizeof(int));
  ret[0] = cntx->f_width;
  ret[1] = cntx->f_height;
  return ret;
}

double getRatio(void) {
  return cntx->ratio;
}

int *getFpsList(void) {
  int *ret = malloc(500 * sizeof(int));
  unsigned int i;
  for (i=0; i<500; ++i) {
    ret[i] = cntx->fps_list[i];
  }
  return ret; 
}

int *getDefFps(void) {
  int *ret = malloc(2 * sizeof(int));
  ret[0] = cntx->d_framerate.num;
  ret[1] = cntx->d_framerate.den;
  return ret;
}

int *getFps(void) {
    int *ret = malloc(2 * sizeof(int));
  ret[0] = cntx->framerate.num;
  ret[1] = cntx->framerate.den;
  return ret;
}

int getStrFps(void) {
  return c_cntx->framerate;
}

void setFilename(const char *s) {
  strncpy(c_cntx->filename, s, 50);
}

void setCRF(int crf) {
  c_cntx->crf = crf;
}

void setMuxer(const char *s) {
  strncpy(c_cntx->format, s, 20);
}

void setTune(const char *s) {
  strncpy(c_cntx->tune, s, 11);
}

void setPreset(const char *s) {
  strncpy(c_cntx->preset, s, 9);
}

unsigned int getArea(void) {
  return c_cntx->area;
}

unsigned int getQArea(void) {
  return c_cntx->q_area;
}

char *getFilename(void) {
  return c_cntx->filename;
}

int getCRF(void) {
  return c_cntx->crf;
}

char *getMuxer(void) {
  return c_cntx->format;
}

char *getTune(void) {
  return c_cntx->tune;
}

char *getPreset(void) {
  return c_cntx->preset;
}

uint8_t *getRGBData(void) {
  return c_cntx->rgb_dst[0];
}

void initCtx(void) {
  cntx = init_ctx();
}

void openDevice(void) {
  open_device(cntx);
}

void initDevice(void) {
  init_device(cntx);
}

void initConvCtx(void) {
  c_cntx = conv_ctx_init(cntx);
}

void startCapture(void) {
  start_capture(cntx);
}

void initRGB(void) {
  rgb24_init(cntx, c_cntx);
}

void initScaler(void) {
  scaler_init(cntx, c_cntx);
}

void initMuxEnc(void) {
  mux_encoder_init(cntx, c_cntx);
}

void capTimeout(void) {
  capture_timeout(cntx);
}

int readFrame(void) {
  if (read_frame(cntx)) {
    return 1;
  }
  return 0;
}

void RGB(void) {
  rgb24(cntx, c_cntx);
}

void _scale(void) {
  scale(cntx, c_cntx);
}

void muxEncode(void) {
  mux_encode(cntx, c_cntx);
}

void writeCache(void) {
  write_cached(cntx, c_cntx);
}

void uninitRGB(void) {
  rgb24_uninit(cntx, c_cntx);
}

void uninitScaler(void) {
  scaler_uninit(cntx, c_cntx);
}

void uninitMuxEnc(void) {
  mux_encoder_uninit(cntx, c_cntx);
}

void uninitConvCtx(void) {
  conv_ctx_uninit(c_cntx);
}

void stopCapture(void) {
  stop_capture(cntx);
  uninit_device(cntx);
}

void closeDevice(void) {
  close_device(cntx);
}

void closeLog(void) {
  close_log();
}
