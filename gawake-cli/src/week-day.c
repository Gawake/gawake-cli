/* wday.c
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

#include "week-day.h"

// Receives a week day from 0 to 13, and returns from 0 to 6 (Sunday to Saturday);
// in other words, two weeks must be represented from 0 to 6 instead of 0 to 13
int week_day (int num)
{
  switch(num)
  {
  case 0:
  case 7:
    return 0;

  case 1:
  case 8:
    return 1;

  case 2:
  case 9:
    return 2;

  case 3:
  case 10:
    return 3;

  case 4:
  case 11:
    return 4;

  case 5:
  case 12:
    return 5;

  case 6:
  case 13:
    return 6;
  }

  return -1;
}
