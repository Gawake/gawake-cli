/* debugger.c
 *
 * Copyright 2021-2024 Kelvin Novais
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "debugger.h"
#include "get-time.h"
#include <time.h>

// Defining a size that's big enough to receive the timesatmp without compilation problems
#define TIMESTAMP_ALLOC 26

const char *print_timestamp (void)
{
  static struct tm * timeinfo;
  static char timestamp[TIMESTAMP_ALLOC];
  get_time_tm (&timeinfo);
  strftime (timestamp, TIMESTAMP_ALLOC, "%Y/%m/%d %H:%M:%S", timeinfo);

  return timestamp;
}
