/**
 * This is part of v4lcapture library.
 * Utility functions.
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

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "struct.h"

int xioctl(int fh, int request, void *arg) {
  int ret;
  
  do {
    ret = ioctl(fh, request, arg);
  } while (-1 == ret && EINTR == errno);

  return ret;
}

int check(int to_check, int source) {
  if (-1 != to_check && source != to_check) {
    return 1;
  }
  return 0;
}
