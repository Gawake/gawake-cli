/* validate-rtcwake-args.c
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

#include "validate-rtcwake-args.h"
#include "debugger.h"
#include "get-time.h"

int validade_rtcwake_args (RtcwakeArgs *rtcwake_args)
{
  DEBUG_PRINT (("Validating rtcwake_args..."));

  gboolean hour, minutes, date, year, mode;
  hour = minutes = date = year  = mode = FALSE;
  int ret;
  struct tm *timeinfo;

  // Hour
  if (rtcwake_args->hour >= 0 && rtcwake_args->hour <= 23)
    hour = TRUE;

  // Minutes
  switch (rtcwake_args->minutes)
    {
    case M_00:
    case M_10:
    case M_20:
    case M_30:
    case M_40:
    case M_50:
      minutes = TRUE;
      break;

    default:
      minutes = FALSE;
    }

  // Date
  struct tm input = {
    .tm_mday = rtcwake_args->day,
    .tm_mon = rtcwake_args->month - 1,
    .tm_year = rtcwake_args->year - 1900,
  };
  time_t generated_time = mktime (&input);
  timeinfo = localtime (&generated_time);
  if (generated_time == -1
      || rtcwake_args->day != timeinfo->tm_mday
      || rtcwake_args->month != timeinfo->tm_mon + 1
      || rtcwake_args->year != timeinfo->tm_year + 1900)
    date = FALSE;
  else
    date = TRUE;

  // Year (must be this year or at most the next, only)
  get_time_tm (&timeinfo);
  if (rtcwake_args->year > (timeinfo->tm_year + 1900 + 1))
    year = FALSE;
  else
    year = TRUE;

  switch (rtcwake_args->mode)
    {
    case MEM:
    case DISK:
    case OFF:
      mode = TRUE;
      break;

    default:
      mode = FALSE;
    }

  if (hour && minutes && year && mode)
    ret = 1;    // valid
  else
    ret = -1;   // invalid

  DEBUG_PRINT (("RtcwakeArgs validation:\n"\
                "\tHour: %d\n\tMinutes: %d\n\tDate: %d\n\tYear: %d\n"\
                "\tMode: %d\n\tthis_year: %d\n\t--> Passed: %d",
                hour, minutes, date, year, mode, timeinfo->tm_year + 1900, ret));

  return ret;
}
