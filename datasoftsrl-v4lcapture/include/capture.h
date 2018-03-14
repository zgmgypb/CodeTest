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

#include <linux/videodev2.h> /* video4linux2 library */

#include "struct.h"

/**
 * Creates a context with default values applied.
 * 
 *> context: pointer to ctx with default values.
 */
ctx *init_ctx();

/**
 * Open provided V4L2 device in r/w mode, with error checking.
 * 
 *< cntx: context
 */
void open_device(ctx *cntx);

/**
 * Crops output as requested into context.
 *
 *< cntx: context
 */
void v4l2_crop(ctx *cntx);

/**
 * Set format of output in terms of size and colorspace.
 *
 *< cntx: context
 *
 *> sizeimage: image size in bytes (to define buffer size)
 */
int v4l2_format(ctx *cntx);

/**
 * Set framerate of output in terms of frames per second.
 *
 *< cntx: context
 */
int v4l2_framerate(ctx *cntx);

/**
 * Initialize device, queries v4l2 for capabilities, video capture, read/write,
 * streaming, crop and set format of output and finally choose right function
 * to start according to hardware.
 * 
 *< cntx: context
 */
void init_device(ctx *cntx);

/**
 * Prepare and initialize buffer and marks device as active.
 * 
 *< cntx: context
 */
void start_capture(ctx *cntx);

/**
 * Set read timeout of two seconds and cycle.
 * 
 *< cntx: context
 */
void capture_timeout(ctx *cntx);

/**
 * Fetch frame and the process it to obtain an image.
 *
 *< cntx: context
 *
 *> 0 if successful, 1 otherwise (even in the end)
 */
int read_frame(ctx *cntx);

/**
 * Stop capturing from device and mark it inactive.
 * 
 *< cntx: context
 */
void stop_capture(ctx *cntx);

/**
 * Free and unallocate memory used for buffers.
 *
 *< cntx: context
 */
void uninit_device(ctx *cntx);

/**
 * Close the previously opened device.
 */
void close_device(ctx *cntx);
