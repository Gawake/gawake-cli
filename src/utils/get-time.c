/* get-time.c
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

// Get the system time (now) as struct tm
int get_time_tm (struct tm **timeinfo)
{
  time_t rawtime;
  time (&rawtime);

  // If fails
  if (rawtime == (time_t) -1)
    {
      fprintf (stderr, "ERROR: failed while getting time\n");
      return EXIT_FAILURE;
    }

  *timeinfo = localtime (&rawtime);
  return EXIT_SUCCESS;
}

// Get the system time (now) as time_t
int get_time (time_t *time_now)
{
  time (time_now);
  if (*time_now ==  (time_t) -1)
    {
      fprintf (stderr, "ERROR: failed while getting time\n");
      return EXIT_FAILURE;
    }
  else
    return EXIT_SUCCESS;
}
