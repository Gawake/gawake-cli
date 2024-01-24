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

#include "get-time.h"

void print_timestamp (void)
{
  static struct tm * timeinfo;
  get_time_tm (&timeinfo);
  printf ("[%02d/%02d/%d %02d:%02d:%02d]: ",
          timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}
