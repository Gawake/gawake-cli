/* scheduler.c
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
#include <stdio.h>

#include "scheduler.h"
#include "get-time.h"

int scheduler (pipe_args_t *args)
{
  int rc, now, db_time = 1, id_match = -1, alloc = 230, gawake_stat = 0;
  Mode mode, default_mode;

  struct tm *timeinfo;                // Default structure, see documentation
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  char time[7];
  const char *DB_TIMES[] = {"utc", "localtime"};
  const char *DAYS[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};     // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
  char query[alloc], buffer[7], date[9], options[alloc];

  // OPEN DATABASE
  rc = sqlite3_open_v2 (DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "Couldn't open database: %s\n", sqlite3_errmsg(db));
      return EXIT_FAILURE;
    }

  // GET THE DATABASE CONFIG
  rc = sqlite3_prepare_v2 (db,
                           "SELECT localtime, def_mode, status FROM config WHERE id = 1;",
                           -1,
                           &stmt,
                           NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "ERROR: Failed getting config information\n");
      sqlite3_close (db);
      return EXIT_FAILURE;
    }

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // (boolean) 0 = utc, 1 = localtime
      db_time = sqlite3_column_int (stmt, 0);
      // default mode, for schedules that doesn't come from a turn off rule
      snprintf (mode, 8, "%s", sqlite3_column_text(stmt, 1));
      snprintf(options, 129, "%s", sqlite3_column_text(stmt, 2));       // rtcwake options
      cmd_stat =      sqlite3_column_int(stmt, 3);
      gawake_stat =   sqlite3_column_int(stmt, 4);
    }
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR (failed getting config information): %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // DO NOTHING IF GAWAKE IS DISABLED
  if (gawake_stat == 0) {
    fprintf(stdout, "Gawake is disabled, exiting...\n");
    return EXIT_SUCCESS;
  }












  // GET THE CURRENT TIME
  get_time (&timeinfo);                                                       // hour, minutes and seconds as integer members
  snprintf (buffer, 7, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);  // Concatenate: HHMM as a string
  now = atoi (buffer);                                                        // HHMM as an integer, leading zeros doesn't matter

  return EXIT_SUCCESS;
}
