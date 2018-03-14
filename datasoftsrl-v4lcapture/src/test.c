/**
 * This is part of v4lcapture library.
 * Main thread.
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
#include <errno.h>
#include <stdbool.h> /* boolean type */
#include <string.h>

#include "capture.h"
#include "struct.h"
#include "error.h"
#include "encoding.h"
#include "util.h"

int main(void) {
  open_log(NULL);
  
  ctx *context = init_ctx();
  
  strcpy(context->dev_name, "/dev/video1");
  
  open_device(context);
  
  //printf("%d %d\n", context->d_framerate.num, context->d_framerate.den);
  
  //context->c_top = 0;
  //context->c_left = 0;
  //context->c_width = 960;
  //context->c_height = 540;
  //
  //context->f_width = 960;
  //context->f_height = 540;
  
  //context->framerate.num = 1;
  //context->framerate.den = 7;
  
  init_device(context);
  
  //printf("%d %d\n", context->framerate.num, context->framerate.den);
  
  context->stop = 60;
  bool infinite;
  if (context->stop == 0) {
    infinite = true;
  }
  
  conv_ctx *conv_context = conv_ctx_init(context);
  
  //strcpy(conv_context->filename, "test.ts");
  strcpy(conv_context->format, "avi");
  
  start_capture(context);

  //rgb24_init(context, conv_context);
  
  scaler_init(context, conv_context);
  mux_encoder_init(context, conv_context);

  while (context->stop-- | infinite) {
    for (;;) {
      capture_timeout(context);
      
      /* read frame and go on */
      if (read_frame(context)) {
        //rgb24(context, conv_context);
        scale(context, conv_context);
        mux_encode(context, conv_context);
        break;
      }
    }
  }

  write_cached(context, conv_context);

  //rgb24_uninit(context, conv_context);

  scaler_uninit(context, conv_context);
  mux_encoder_uninit(context, conv_context);

  conv_ctx_uninit(conv_context);

  stop_capture(context);
  uninit_device(context);
  
  close_device(context);
  
  close_log();

  return EXIT_SUCCESS;
}
