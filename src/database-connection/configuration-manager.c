/* configuration-manager.c
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

#include <stdlib.h>
#include <sqlite3.h>

#include "database-connection-utils.h"
#include "configuration-manager.h"

int
configuration_set_localtime (bool use_localtime)
{
  sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                    "UPDATE config "\
                    "SET localtime = %d "\
                    "WHERE id = 1;",
                    use_localtime);

  return utils_run_sql ();
}

int
configuration_set_default_mode (Mode default_mode)
{
  // Validate mode
  if (default_mode >= MODE_MEM && default_mode <= MODE_OFF)
    {
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                        "UPDATE config "\
                        "SET default_mode = %d "\
                        "WHERE id = 1;",
                        default_mode);

      return utils_run_sql ();
    }
  else
    return EXIT_FAILURE;
}

int
configuration_set_notification_time (int notification_time)
{
  if (notification_time >= 0 && notification_time <= MAX_NOTIFICATION_TIME)
    {
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                        "UPDATE config "\
                        "SET notification_time = %d "\
                        "WHERE id = 1;",
                        notification_time);

      return utils_run_sql ();
    }
  else
    return EXIT_FAILURE;
}

int
configuration_set_shutdown_fail (bool shutdown_fail)
{
  sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                    "UPDATE config "\
                    "SET shutdown_fail = %d "\
                    "WHERE id = 1;",
                    shutdown_fail);

  return utils_run_sql ();
}
