/* database-connection-utils.c
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

#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../utils/debugger.h"
#include "database-connection-utils.h"

static sqlite3 *db = NULL;

int
validate_rule (const Rule *rule)
{
  // name can't be bigger than the max value
  bool name = !(strlen (rule->name) >= RULE_NAME_LENGTH);     // (>=) do not include null terminator

  // hour [00,23]
  bool hour = rule->hour <= 23;     // Due to the data type, it is always >= 0

  // minutes [0, 59]
  bool minutes = rule->minutes <= 59;

  // mode according to enum predefined values
  // TODO update enum
  bool mode = (rule->mode >= 0 && rule->mode <= OFF);

  bool table = (rule->table == T_ON || rule->table == T_OFF);

  DEBUG_PRINT (("VALIDATION:\nName: %d\n\tName length: %lu\n"\
                "Hour: %d\nMinutes: %d\nMode: %d\nTable: %d",
                name, strlen (rule->name), hour, minutes, mode, table));

  if (name && hour && minutes && mode && table)
    return EXIT_SUCCESS;
  else
    {
      fprintf (stderr, "Invalid rule values\n\n");
      return EXIT_FAILURE;
    }
}

int
validate_table (const Table table)
{
  if (table == T_ON || table == T_OFF)
    return EXIT_SUCCESS;

  fprintf (stderr, "Invalid table\n\n");
  return EXIT_FAILURE;
}

int
run_sql (void)
{
  int rc;
  char *err_msg = 0;

  if (db == NULL)
    {
      fprintf (stderr, "Database not connected\n");
      return EXIT_FAILURE;
    }

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
  if (rc != SQLITE_OK)
    fprintf (stderr, "Failed to run SQL: %s\n", sqlite3_errmsg(db));

  sqlite3_free (err_msg);

  if (rc == SQLITE_OK)
    {
      // TODO
      /* trigger_update_database (); */
      return EXIT_SUCCESS;
    }
  else
    return EXIT_FAILURE;
}

sqlite3 * get_pdb (void)
{
  return db;
}

sqlite3 ** get_ppdb (void)
{
  return &db;
}
