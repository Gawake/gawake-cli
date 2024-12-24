/* configuration-reader.c
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

#include "../utils/debugger.h"
#include "database-connection-utils.h"
#include "configuration-reader.h"

typedef struct
{
  bool use_localtime;
  Mode default_mode;
  int notification_time;
  bool shutdown_fail;
} Config;

static Config config =
{
  false,
  MODE_OFF,
  NOTIFICATION_TIME_00,
  false
};

static int
get_config (void)
{
  // Database related variables
  int rc;
  struct sqlite3_stmt *stmt;

  // Generate SQL
  sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                    "SELECT * FROM config "\
                    "WHERE id = 1;");

  DEBUG_PRINT (("Generated SQL:\n\t%s", utils_get_sql ()));

  if (sqlite3_prepare_v2 (utils_get_pdb (), utils_get_sql (), -1, &stmt, NULL) != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Failed to query rule\n");
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // Note: column 0 is the id
      // Note: column 1 is the cli_version

      // Use localtime
      config.use_localtime = (bool) sqlite3_column_int (stmt, 2);

      // Default mode
      config.default_mode = (Mode) sqlite3_column_int (stmt, 3);

      // Notification time
      config.notification_time = sqlite3_column_int (stmt, 4);

      // Shutdown if fails
      config.shutdown_fail = (bool) sqlite3_column_int (stmt, 5);
    }

  if (rc != SQLITE_DONE)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR (failed to query rule): %s\n"), sqlite3_errmsg (utils_get_pdb ());
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize(stmt);

  return EXIT_SUCCESS;
}

int
configuration_get_localtime (bool *use_localtime)
{
  int ret = get_config ();
  *use_localtime = config.use_localtime;
  return ret;
}

int
configuration_get_default_mode (Mode *default_mode)
{
  int ret = get_config ();
  *default_mode = config.default_mode;
  return ret;
}

int
configuration_get_notification_time (int *notification_time)
{
  int ret = get_config ();
  *notification_time = config.notification_time;
  return ret;
}

int
configuration_get_shutdown_fail (bool *shutdown_fail)
{
  int ret = get_config ();
  *shutdown_fail = config.shutdown_fail;
  return ret;
}
