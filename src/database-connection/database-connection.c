/* database-connection.c
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

#include <stdio.h>

#include "database-connection.h"
#include "database-connection-utils.h"
#include "../utils/debugger.h"

// This function connect to the database
// Should be called once
int
connect_database (bool read_only)
{
  int rc = 0;

  if (utils_get_pdb () != NULL)
    {
      printf ("Warning: Database already connected.\n");
      return SQLITE_OK;
    }

  // Open the SQLite database
  if (!read_only)
    rc = sqlite3_open_v2 (DB_PATH, utils_get_ppdb (), SQLITE_OPEN_READWRITE, NULL);
  else
    rc = sqlite3_open_v2 (DB_PATH, utils_get_ppdb (), SQLITE_OPEN_READONLY, NULL);

  if (rc != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (utils_get_pdb ()));
      sqlite3_close (utils_get_pdb ());
      return rc;
    }
  else
    {
      // Enable security options
      sqlite3_db_config (utils_get_pdb (), SQLITE_DBCONFIG_DEFENSIVE, 0, 0);
      sqlite3_db_config (utils_get_pdb (), SQLITE_DBCONFIG_ENABLE_TRIGGER, 0, 0);
      sqlite3_db_config (utils_get_pdb (), SQLITE_DBCONFIG_ENABLE_VIEW, 0, 0);
      sqlite3_db_config (utils_get_pdb (), SQLITE_DBCONFIG_TRUSTED_SCHEMA, 0, 0);
    }

  return rc;
}

int
disconnect_database (void)
{
  sqlite3 **db = utils_get_ppdb ();
  int rc = sqlite3_close (utils_get_pdb ());
  *db = NULL;
  return rc;
}
