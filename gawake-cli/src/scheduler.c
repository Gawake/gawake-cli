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
#include <unistd.h>
#include <pthread.h>

#include "scheduler.h"
#include "get-time.h"
#include "week-day.h"

// TODO static functions
void *dbus_listener ();
void *timed_checker ();
int day_changed (void);
int query_upcoming_off_rule (UpcomingOffRule *upcoming_off_rule);
int query_upcoming_on_rule (RtcwakeArgs *rtcwake_args);
// TODO
int validade_rtcawake_args (RtcwakeArgs *rtcwake_args);

static volatile gboolean database_updated = FALSE, cancel = FALSE;

int scheduler (RtcwakeArgs *args)
{
  pthread_t timed_checker_thread, dbus_listener_thread;

  if (pthread_create (&dbus_listener_thread, NULL, &dbus_listener, NULL) != 0)
    fprintf (stderr, "ERROR: Failed to create dbus_listener thread\n");

  if (pthread_create (&timed_checker_thread, NULL, &timed_checker, NULL) != 0)
    fprintf (stderr, "ERROR: Failed to create timed_checker thread\n");

  return EXIT_SUCCESS;
}

// Thread 1: listen to D-Bus signals
void *dbus_listener ()
{
  // Database updated
  // Immediate schedule (check if it's a custom schedule or a signal to trigger next rule): call pthread_kill()
  // Cancel schedule

  return NULL;
}

/* Thread 2: periodically make a check:
 * -> if there's a turn off upcoming rule, calculate how much time lacks to
 * to run that rule; also notify the user;
 *
 * -> if there's NOT a turn off for today, check if there's a new one when the
 * day changes or the database is updated
 */
void *timed_checker ()
{
  UpcomingOffRule upcoming_off_rule;

  // Check rules for today, however, keep track of which day is today:
  // I'm considering the  possibility of the device be active across days)
  query_upcoming_off_rule (&upcoming_off_rule);

  TIMED_CHECKER_LOOP:
  while (1)
    {
      /* if (no_rule && day_changed) */
      /*   update_upcoming_rule (&ur); */

      /* if (database_updated) */
      /*   { */
      /*     update_upcoming_rule (&ur); */
      /*     database_updated = 0; */
      /*   } */

      /* calculate_timediff */
      /* if (timediff <= 15) */
      /*   break; */

      sleep (CHECK_DELAY);
    }

    /* emit notification missing 5 mintes */
    /* if (cancelled) */
    /*   goto TIMED_CHECKER_LOOP; */

  return NULL;
}

// Checks if the day of the week was changed and returns a boolean
int day_changed (void)
{
  // Keeps the last checked week day; starts is initialized as -1, so the function
  // always returns true on the first call; this behaviour is wanted
  static int last_week_day = -1;

  // Note: this structure is statically allocated, no need to free
  struct tm *timeinfo;

  // Get current time
  get_time (&timeinfo);

  if (timeinfo -> tm_wday != last_week_day)
    return 1;   // day changed
  else
    return 0;   // day not changed
}

int query_upcoming_off_rule (UpcomingOffRule *upcoming_off_rule)
{
  int rc, now, ruletime;

  struct tm *timeinfo;
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  // Index(0 to 6) matches tm_wday; these strings refer to SQLite columns name
  const char DAYS[7][3] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
  char query[ALLOC], buffer[BUFFER_ALLOC], date[9];

  // SET UPCOMING RULE AS NOT FOUND
  upcoming_off_rule -> found = FALSE;

  // OPEN DATABASE
  rc = sqlite3_open_v2 (DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "Couldn't open database: %s\n", sqlite3_errmsg (db));
      return EXIT_FAILURE;
    }

  // GET THE DATABASE CONFIG
  rc = sqlite3_prepare_v2 (db,
                           "SELECT status, notification_time "\
                           "FROM config WHERE id = 1;",
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
      // Gawake status (boolean)
      upcoming_off_rule -> gawake_status = sqlite3_column_int (stmt, 0);

      // Notification time
      upcoming_off_rule -> notification_time = (NotificationTime) sqlite3_column_int (stmt, 1);
    }
  if (rc != SQLITE_DONE)
    {
      fprintf (stderr, "ERROR (failed getting config information): %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  sqlite3_finalize (stmt);

  // GET THE CURRENT TIME
  // hour, minutes and seconds as integer members
  get_time (&timeinfo);
  // Concatenate: HHMM as a string
  snprintf (buffer, BUFFER_ALLOC, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);
  // HHMM as an integer, leading zeros doesn't matter
  now = atoi (buffer);

  // QUERY TURN OFF RULES
  // This query SQL returns time on format HHMM
  snprintf (query,
            ALLOC,
            "SELECT strftime('%%H%%M', rule_time), mode"\
            "FROM rules_turnoff "\
            "WHERE %s = 1 ORDER BY time(rule_time) ASC;",
            DAYS[timeinfo -> tm_wday]);

  rc = sqlite3_prepare_v2 (db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "ERROR: Failed querying turn off rules for today\n");
      sqlite3_close (db);
      return EXIT_FAILURE;
    }

  // Get all rules today, ordered by time:
  // the first rule that is bigger than now is a valid rule
  int hour, minutes;
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      ruletime = sqlite3_column_int (stmt, 0);
      if (ruletime > now)
        {
          // Set "rule found?" to true
          upcoming_off_rule -> found = TRUE;

          // Hour and minutes
          // TODO sqlite3_column_text16 has correct data type for sscanf, but returned data is wrong
          sscanf (sqlite3_column_text (stmt, 0), "%02d%02d", &hour, &minutes);
          upcoming_off_rule -> hour = hour;
          upcoming_off_rule -> minutes = (Minutes) minutes;

          // Mode
          upcoming_off_rule -> mode = (Mode) sqlite3_column_int (stmt, 0);

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

  return EXIT_SUCCESS;
}

int query_upcoming_on_rule (RtcwakeArgs *rtcwake_args)
{
  int rc, now, ruletime, is_localtime = 1, id_match = -1;

  struct tm *timeinfo;
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  // Index(0 to 6) matches tm_wday; these strings refer to SQLite columns name
  const char DAYS[7][3] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
  char query[ALLOC], buffer[BUFFER_ALLOC], date[9];

  // OPEN DATABASE
  rc = sqlite3_open_v2 (DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK)
    {
      fprintf (stderr, "Couldn't open database: %s\n", sqlite3_errmsg (db));
      return EXIT_FAILURE;
    }

  // GET THE DATABASE CONFIG
  rc = sqlite3_prepare_v2 (db,
                           "SELECT localtime, default_mode "\
                           "FROM config WHERE id = 1;",
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
      // Localtime (boolean)
      is_localtime = sqlite3_column_int (stmt, 0);

      // Default mode
      rtcwake_args -> mode = (Mode) sqlite3_column_int (stmt, 1);
    }
  if (rc != SQLITE_DONE)
    {
      fprintf (stderr, "ERROR (failed getting config information): %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  sqlite3_finalize (stmt);

  // GET THE CURRENT TIME
  // hour, minutes and seconds as integer members
  get_time (&timeinfo);
  // Concatenate: HHMM as a string
  snprintf (buffer, BUFFER_ALLOC, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);
  // HHMM as an integer, leading zeros doesn't matter
  now = atoi (buffer);

  fprintf (stdout, "Trying to get schedule for today\n");

  // Create an SQL statement to get today's active rules time; tm_wday = number of the week
  snprintf (query,
            ALLOC,
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
              fprintf (stderr, "ERROR: Failed to get schedule for for tomorrow or later (on wday function)\n");
              sqlite3_close (db);
              return EXIT_FAILURE;
            }

          /*
           * The first rule after today is a valid match;
           * also calculate the day: now + number of day until the matching rule,
           * represented by the index i of the loop
           */
          snprintf (query,
                    ALLOC,
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
              snprintf (date, 9, "%s", sqlite3_column_text (stmt, 1)); // YYYYMMDD
              snprintf (buffer,
                        BUFFER_ALLOC,
                        "%s",
                        sqlite3_column_text (stmt, 2)); // HHMM
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

  // IF ANY RULE WAS FOUND, RETURN AS RULE NOT FOUND
  if (id_match < 0)
    {
      fprintf(stdout, "WARNING: Any turn on rule found.\n");
      rtcwake_args -> found = FALSE;
      return EXIT_SUCCESS;
    }

  // ELSE, RETURN PARAMETERS
  rtcwake_args -> found = TRUE;

  sscanf (date, "%04d%02d%02d",
          &(rtcwake_args -> year),
          &(rtcwake_args -> month),
          &(rtcwake_args -> day));

  return EXIT_SUCCESS;
}
