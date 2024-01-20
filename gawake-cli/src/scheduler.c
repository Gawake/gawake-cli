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
#include "week-day.h"

int scheduler (pipe_args_t *args)
{
  int rc, now, is_localtime = 1, id_match = -1, alloc = 230, gawake_status = 0, ruletime, match = 0;
  Mode mode, default_mode;

  // TODO free?
  struct tm *timeinfo;                // Default structure, see documentation
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
  const char DAYS[7][3] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
  char query[alloc], buffer[BUFFER_ALLOC], date[9], time[7];

  // OPEN DATABASE
  rc = sqlite3_open_v2 (DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "Couldn't open database: %s\n", sqlite3_errmsg (db));
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
      is_localtime = sqlite3_column_int (stmt, 0);
      // default mode, for schedules that doesn't come from a turn off rule
      switch (sqlite3_column_int (stmt, 1))
        {
        case MEM:
          mode = MEM;
          break;

        case DISK:
          mode = DISK;
          break;

        case OFF:
          mode = OFF;
          break;

        default:
          fprintf (stderr, "ERROR: unknown mode\n");
          return EXIT_FAILURE;
        }
      // gawake status (boolean)
      gawake_status = sqlite3_column_int (stmt, 2);
    }
  if (rc != SQLITE_DONE)
    {
      fprintf (stderr, "ERROR (failed getting config information): %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  sqlite3_finalize (stmt);

  // TODO DO NOTHING IF GAWAKE IS DISABLED
  /* if (gawake_status == 0) { */
  /*   fprintf(stdout, "Gawake is disabled, exiting...\n"); */
  /*   return EXIT_SUCCESS; */
  /* } */














  // GET THE CURRENT TIME
  // hour, minutes and seconds as integer members
  get_time (&timeinfo);
  // Concatenate: HHMM as a string
  snprintf (buffer, BUFFER_ALLOC, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);
  // HHMM as an integer, leading zeros doesn't matter
  now = atoi (buffer);

  // CHECK IF THERE IS A TURN OFF RULE WITHIN NEXT 15 MINUTES
  match = 0;
  // Returns time on format HHMM
  snprintf (query,
            alloc,
            "SELECT strftime('%%H%%M', rule_time) FROM rules_turnoff "\
            "WHERE %s = 1 ORDER BY time(rule_time) ASC;",
            DAYS[timeinfo -> tm_wday]);

  rc = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "ERROR: Failed scheduling for today\n");
      sqlite3_close (db);
      return EXIT_FAILURE;
    }

  // Get all rules today, ordered by time:
  // the first rule that is within the next 15 minutes is a valid match
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
      ruletime = sqlite3_column_int (stmt, 0);
      if ((ruletime >= now)
          && (ruletime <= (now + MINUTES_AHEAD)))
        {
          match = 1;
          break;
        }
    }

  if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
      fprintf (stderr, "ERROR (failed while querying rules time): %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  sqlite3_finalize (stmt);

  // ON MATCH
  if (match)
    {
      fprintf (stdout, "Rule matched: trying to schedule for today\n");

      // Create an SQL statement to get today's active rules time; tm_wday = number of the week
      snprintf (query,
                alloc,
                "SELECT id, strftime('%%H%%M', rule_time), strftime('%%Y%%m%%d', 'now', '%s') "\
                "FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC;",
                is_localtime ? "localtime" : "utc",
                DAYS[timeinfo -> tm_wday]);

      rc = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
      if (rc != SQLITE_OK)
        {
          fprintf (stderr, "ERROR: Failed while querying rules to make schedule for today\n");
          sqlite3_close (db);
          return EXIT_FAILURE;
        }

      // Get all rules today, ordered by time; the first rule that has a bigger time than now is a valid
      while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
        {
          int id =        sqlite3_column_int (stmt, 0);
          ruletime =      sqlite3_column_int (stmt, 1);
          if (now < ruletime)
            {
              id_match = id;
              snprintf (date, 9, "%s", sqlite3_column_text (stmt, 2));               // YYYYMMDD
              snprintf (buffer, BUFFER_ALLOC, "%s", sqlite3_column_text (stmt, 1));  // HHMM
              break;
            }
        }
      if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
          fprintf (stderr, "ERROR (failed scheduling for today): %s\n", sqlite3_errmsg (db));
          sqlite3_close (db);
          return EXIT_FAILURE;
        }
      sqlite3_finalize (stmt);

      // IF IT WASN'T POSSIBLE TO SCHEDULE FOR TODAY, TRY ON THE NEXT DAYS
      if (id_match < 0)
        {
          fprintf (stdout, "Any time matched. Trying to schedule for tomorrow or later\n");
          // search for a matching rule within a week
          for (int i = 1; i <= 7; i++)
            {
              int wday_num = week_day (timeinfo -> tm_wday + i);
              if (wday_num == -1)
                {
                  fprintf (stderr, "ERROR: Failed scheduling for after (on wday function)\n");
                  sqlite3_close (db);
                  return EXIT_FAILURE;
                }

              // The first rule after today is a valid match;
              // also calculate the day: now + number of day until the matching rule, represented by the index i of the loop
              snprintf (query,
                        alloc,
                        "SELECT id, strftime('%%Y%%m%%d', 'now', '%s', '+%d day'), strftime('%%H%%M', time) "\
                        "FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC LIMIT 1;",
                        is_localtime ? "localtime" : "utc",
                        i,
                        DAYS[wday_num]);

              rc = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
              if (rc != SQLITE_OK)
                {
                  fprintf (stderr, "ERROR: Failed scheduling for after\n");
                  return EXIT_FAILURE;
                }
              while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
                {
                  id_match = sqlite3_column_int (stmt, 0);
                  snprintf (date, 9, "%s", sqlite3_column_text (stmt, 1));              // YYYYMMDD
                  snprintf (buffer, BUFFER_ALLOC, "%s", sqlite3_column_text (stmt, 2)); // HHMM
                }
              if (rc != SQLITE_DONE)
                {
                  fprintf (stderr, "ERROR (failed scheduling for after): %s\n", sqlite3_errmsg (db));
                  sqlite3_close (db);
                  return EXIT_FAILURE;
                }
              sqlite3_finalize (stmt);

              if (id_match >= 0)
                break;

            }
        }

      // IF ANY RULE WAS FOUND, RETURN
      if (id_match < 0)
        {
          fprintf(stdout, "WARNING: Any turn on rule found. Schedule failed!\n");
          /* return EXIT_SUCCESS; */
        }
    }

  return EXIT_SUCCESS;
}
