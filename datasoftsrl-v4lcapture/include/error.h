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

#include <stdarg.h> /* variable paramaters function */

/**
 * Prints out a generic error and exits in case of failure.
 *
 *< s: error format
 *< ...: arguments
 */
void gen_critical(const char *s, ...);

/**
 * Prints the type, number and description  of the capability error,
 * and exits in case of failure.
 *
 *< s: type of the error
 */
void cap_critical(const char *s);

/**
 * Prints out error related to the physical device (eg. not supporting
 * capability) and exits in case of failure.
 *
 *< dev_name: name of the physical device
 *< s: error description
 */
void dev_critical(const char *dev_name, const char *s);

/**
 * Prints out a generic error.
 *
 *< s: error format
 *< ...: arguments
 */
void gen_error(const char *s, ...);

/**
 * Prints the type, number and description  of the capability error.
 *
 *< s: type of the error
 */
int cap_error(const char *s);

/**
 * Prints out error related to the physical device (eg. not supporting
 * capability).
 *
 *< dev_name: name of the physical device
 *< s: error description
 */
int dev_error(const char *dev_name, const char *s);

/**
 * Prints out a generic error.
 *
 *< s: error format
 *< ...: arguments
 */
void gen_log(const char *s, ...);

/**
 * Prints the type, number and description  of the capability error,
 * and returns number.
 *
 *< s: type of the error
 *
 *> errno: code of error
 */
int cap_log(const char *s);

/**
 * Prints out error related to the physical device (eg. not supporting
 * capability) and returns error code.
 *
 *< dev_name: name of the physical device
 *< s: error description
 *
 *> errno: code of error
 */
int dev_log(const char *dev_name, const char *s);

/**
 * Closes log file.
 * 
 *< logfd: opened log file.
 */
void close_log();
