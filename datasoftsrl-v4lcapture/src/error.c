/**
 * This is part of v4lcapture library.
 * Error management functions.
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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h> /* variable number of parameters */

#include <syslog.h>

#include "error.h"
#include "util.h"

void gen_critical(const char *s, ...) {
  /* set log metadata */
  openlog("libv4lcapture", LOG_CONS || LOG_PID, LOG_USER);
  
  /* variable args */
  va_list args;
  
  /* actually print */
  va_start(args, s);
  vsyslog(LOG_USER || LOG_CRIT, s, args);
  va_end(args);

  exit(EXIT_FAILURE);
}

void cap_critical(const char *s) {
  gen_critical("error: %s (%d): %s\n", s, errno, strerror(errno));

  exit(errno);
}

void dev_critical(const char *dev_name, const char *s) {
  gen_critical("%s %s\n", dev_name, s);

  exit(errno);
}

void gen_error(const char *s, ...) {
  /* set log metadata */
  openlog("libv4lcapture", LOG_CONS || LOG_PID, LOG_USER);
  
  /* variable args */
  va_list args;
  
  /* actually print */
  va_start(args, s);
  vsyslog(LOG_USER || LOG_ERR, s, args);
  va_end(args);
}

int cap_error(const char *s) {
  gen_error("error: %s (%d): %s\n", s, errno, strerror(errno));

  return errno;
}

int dev_error(const char *dev_name, const char *s) {
  gen_error("%s %s\n", dev_name, s);

  return errno;
}

void gen_log(const char *s, ...) {
  /* set log metadata */
  openlog("libv4lcapture", LOG_CONS || LOG_PID, LOG_USER);
  
  /* variable args */
  va_list args;
  
  /* actually print */
  va_start(args, s);
  vsyslog(LOG_USER || LOG_INFO, s, args);
  va_end(args);
}

int cap_log(const char *s) {
  gen_log("error: %s (%d): %s\n", s, errno, strerror(errno));

  return errno;
}

int dev_log(const char *dev_name, const char *s) {
  gen_log("%s %s\n", dev_name, s);

  return errno;
}

void close_log() {
  closelog();
}
