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

#include "struct.h"

#ifndef CLEAR_H
#define CLEAR_H

#define clear(x) memset(&(x), 0, sizeof(x)) /* struct cleaner */

#endif /* CLEAR_H */

/**
 * Wrapper for the ioctl system call, that cycles until -1 or EINTR errors.
 *
 *< fh: file descriptor of V4l2 device
 *< request: the specified ioctl request
 *< *arg: optional arguments (such as struct to fill)
 *
 *> ret: status of ioctl
 */
int xioctl(int fh, int request, void *arg);

/**
 * Check that to_check int is not -1 and is different from source.
 * 
 *< to_check: integer to check validity
 *< source: integer to check against
 *
 *> 1 if valid, 0 otherwise
 */
int check(int to_check, int source);
