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

// FIXME possible problem: user won't get notified if the turn off rule is tomorrow
// but that notification should happen today.
// In this case the device is active across days

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "scheduler.h"
#include "get-time.h"
#include "week-day.h"
#include "debugger.h"

static void *dbus_listener (void *args);
static void *timed_checker (void *args);
static int day_changed (void);
static int query_upcoming_off_rule (void);  // TODO treat on fail (?)
static int query_upcoming_on_rule (void);   // TODO treat on fail (?)
static void sync_time (void);

/*
 * These are static (and private to this file) variables.
 * They are shared among:
 * -> timed_checker and dbus_listener threads;
 * -> functions in this file that are called by the threads;
 *
 * These variables shouldn't be used on other files.
 */
static volatile gboolean database_updated = FALSE, cancel = FALSE;
static UpcomingOffRule upcoming_off_rule;

// ATTENTION: This is a POINTER to a gawaked.c OWNED variable
static RtcwakeArgs *rtcwake_args;

static pthread_mutex_t upcoming_off_rule_mutex, rtcwake_args_mutex, booleans_mutex;

int scheduler (RtcwakeArgs *rtcwake_args_ptr)
{
  rtcwake_args = rtcwake_args_ptr;
  pthread_t timed_checker_thread, dbus_listener_thread;

  pthread_mutex_init (&upcoming_off_rule_mutex, NULL);
  pthread_mutex_init (&rtcwake_args_mutex, NULL);
  pthread_mutex_init (&booleans_mutex, NULL);

  // CREATE THREADS
  if (pthread_create (&dbus_listener_thread, NULL, &dbus_listener, NULL) != 0)
    {
      fprintf (stderr, "ERROR: Failed to create dbus_listener thread\n");
      return EXIT_FAILURE;
    }
  if (pthread_create (&timed_checker_thread, NULL, &timed_checker, rtcwake_args) != 0)
    {
      fprintf (stderr, "ERROR: Failed to create timed_checker thread\n");
      return EXIT_FAILURE;
    }

  // JOIN THREADS
  if (pthread_join (timed_checker_thread, NULL) != 0)
    {
      fprintf (stderr, "ERROR: Failed to join timed_checker thread\n");
      return EXIT_FAILURE;
    }
  if (pthread_join (dbus_listener_thread, NULL) != 0)
    {
      fprintf (stderr, "ERROR: Failed to join dbus_listener thread\n");
      return EXIT_FAILURE;
    }

  pthread_mutex_destroy (&upcoming_off_rule_mutex);
  pthread_mutex_destroy (&rtcwake_args_mutex);
  pthread_mutex_destroy (&booleans_mutex);

  return EXIT_SUCCESS;
}

// Thread 1: listen to D-Bus signals
static void *dbus_listener (void *args)
{
  DEBUG_PRINT (("Started dbus_listener thread"));

  // Database updated
  sleep (50);

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
static void *timed_checker (void *args)
{
  DEBUG_PRINT (("Started timed_checker thread"));
  double diff;

  // Check rules for today, however, keep track of which day is today:
  // I'm considering the  possibility of the device be active across days)
  query_upcoming_off_rule ();

  sync_time ();

  TIMED_CHECKER_LOOP:
  while (1)
    {
      DEBUG_PRINT_TIME (("New lap on timed_checker main loop"));

      if (!upcoming_off_rule.found && day_changed ())
        {
          DEBUG_PRINT (("Querying rule because day changed"));
          query_upcoming_off_rule ();
        }

      if (database_updated)
        {
          DEBUG_PRINT (("Querying rule because database updated"));
          query_upcoming_off_rule ();
          pthread_mutex_lock (&booleans_mutex);
          database_updated = FALSE;
          pthread_mutex_unlock (&booleans_mutex);
        }

      // Calculate time until the upcoming off rule
      if (upcoming_off_rule.found)
        {
          time_t now;
          get_time (&now);
          diff = difftime (upcoming_off_rule.rule_time, now);    // seconds

          DEBUG_PRINT (("Missing time: %f s", diff));

          /*
           * If the time missing is lesser than the delay plus notification plus
           *  minimum sync time, exit the loop
           */
          if (diff <= (CHECK_DELAY + upcoming_off_rule.notification_time + 60))
            break;
        }

      sleep (CHECK_DELAY);
    }

  // TODO get turn on rule attributes; override default mode with mode

  sync_time ();

  // Emit notification missing X minutes
  sleep (diff - upcoming_off_rule.notification_time);
  DEBUG_PRINT_TIME (("Notification"));
  // TODO notification
  /* if (failed_get_attributes) */
  /*   send error */
  /* else */
  /*   send notification */

  sleep (upcoming_off_rule.notification_time);

  /* if (cancelled) */
  /*   goto TIMED_CHECKER_LOOP; */
  /* else */
  /*   schedule */
  /*   on fail, notify and shutdown */

  DEBUG_PRINT_TIME (("Scheduling"));

  return NULL;
}

// Checks if the day of the week was changed and returns a boolean
static int day_changed (void)
{
  // Keeps the last checked week day; starts is initialized as -1, so the function
  // always returns true on the first call; this behaviour is wanted
  static int last_week_day = -1;
  int ret;

  // Note: this structure is statically allocated, no need to free
  struct tm *timeinfo;

  // Get current time
  get_time_tm (&timeinfo);

  DEBUG_PRINT (("timeinfo->tm_wday: %d; last_week_day: %d", timeinfo -> tm_wday, last_week_day));
  if (timeinfo -> tm_wday != last_week_day)
    ret = 1;   // day changed
  else
    ret =  0;   // day not changed

  // Update the variable
  last_week_day = timeinfo -> tm_wday;

  return ret;
}

static int query_upcoming_off_rule (void)
{
  int rc, now, ruletime;

  struct tm *timeinfo;
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  // Index(0 to 6) matches tm_wday; these strings refer to SQLite columns name
  const char DAYS[7][4] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
  char query[ALLOC], buffer[BUFFER_ALLOC];

  // SET UPCOMING RULE AS NOT FOUND
  pthread_mutex_lock (&upcoming_off_rule_mutex);
  upcoming_off_rule.found = FALSE;
  pthread_mutex_unlock (&upcoming_off_rule_mutex);

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
      pthread_mutex_lock (&upcoming_off_rule_mutex);

      // Gawake status (boolean)
      upcoming_off_rule.gawake_status = sqlite3_column_int (stmt, 0);

      // Notification time
      upcoming_off_rule.notification_time = (NotificationTime) sqlite3_column_int (stmt, 1);
      // Convert to seconds
      upcoming_off_rule.notification_time *= 60;

      pthread_mutex_unlock (&upcoming_off_rule_mutex);
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
  get_time_tm (&timeinfo);
  // Concatenate: HHMM as a string
  snprintf (buffer, BUFFER_ALLOC, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);
  // HHMM as an integer, leading zeros doesn't matter
  now = atoi (buffer);

  // QUERY TURN OFF RULES
  // This query SQL returns time on format HHMM
  snprintf (query,
            ALLOC,
            "SELECT strftime('%%H%%M', rule_time), mode "\
            "FROM rules_turnoff WHERE %s = 1 "\
            "ORDER BY time(rule_time) ASC;",
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
          pthread_mutex_lock (&upcoming_off_rule_mutex);

          // Set "rule found?" to true
          upcoming_off_rule.found = TRUE;

          // Hour and minutes
          // TODO sqlite3_column_text16 has correct data type for sscanf, but returned data is wrong
          sscanf (sqlite3_column_text (stmt, 0), "%02d%02d", &hour, &minutes);
          upcoming_off_rule.hour = hour;
          upcoming_off_rule.minutes = (Minutes) minutes;

          // Fill time_t
          timeinfo -> tm_hour = hour;
          timeinfo -> tm_min = minutes;
          /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           * Note: other fields (day, month, year) on timeinfo were not changed;
           * If there is the need of considering the rule at tomorrow, this might be modified
           */
          upcoming_off_rule.rule_time = mktime (timeinfo);
          // If fails
          if (upcoming_off_rule.rule_time == (time_t) -1)
            return EXIT_FAILURE;

          // Mode
          upcoming_off_rule.mode = (Mode) sqlite3_column_int (stmt, 1);

          pthread_mutex_unlock (&upcoming_off_rule_mutex);
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

  DEBUG_PRINT (("Upcoming rule fields:\n"\
                "\tFound: %d\n\tHour: %02d\n\tMinutes: %02d\n\tMode: %d\n\tNotification time: %d s",
                upcoming_off_rule.found, upcoming_off_rule.hour, upcoming_off_rule.minutes,
                upcoming_off_rule.mode, upcoming_off_rule.notification_time));

  return EXIT_SUCCESS;
}

static int query_upcoming_on_rule (void)
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
      pthread_mutex_lock (&rtcwake_args_mutex);
      rtcwake_args -> mode = (Mode) sqlite3_column_int (stmt, 1);
      pthread_mutex_unlock (&rtcwake_args_mutex);
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
  get_time_tm (&timeinfo);
  // Concatenate: HHMM as a string
  snprintf (buffer, BUFFER_ALLOC, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min);
  // HHMM as an integer, leading zeros doesn't matter
  now = atoi (buffer);

  fprintf (stdout, "Trying to get schedule for today\n");

  // Create an SQL statement to get today's active rules time; tm_wday = number of the week
  snprintf (query,
            ALLOC,
            "SELECT id, strftime('%%H%%M', rule_time), strftime('%%Y%%m%%d', 'now', '%s') "\
            "FROM rules_turnon WHERE %s = 1 ORDER BY time(rule_time) ASC;",
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
          while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
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
      pthread_mutex_lock (&rtcwake_args_mutex);
      rtcwake_args -> found = FALSE;
      pthread_mutex_unlock (&rtcwake_args_mutex);
      return EXIT_SUCCESS;
    }

  // ELSE, RETURN PARAMETERS
  pthread_mutex_lock (&rtcwake_args_mutex);

  rtcwake_args -> found = TRUE;

  int minutes;
  sscanf (buffer, "%02d%02d",
          &(rtcwake_args -> hour),
          &minutes);
  rtcwake_args -> minutes = (Minutes) minutes;

  sscanf (date, "%04d%02d%02d",
          &(rtcwake_args -> year),
          &(rtcwake_args -> month),
          &(rtcwake_args -> day));

  pthread_mutex_unlock (&rtcwake_args_mutex);
  return EXIT_SUCCESS;
}

int validade_rtcwake_args (void)
{
  pthread_mutex_lock (&rtcwake_args_mutex);

  gboolean hour, minutes, year, timestamp, mode;
  hour = minutes = year = timestamp = mode = FALSE;
  time_t time_check;
  int ret;
  struct tm *timeinfo;

  // Hour
  if (rtcwake_args -> hour >= 0 && rtcwake_args -> hour <= 23)
    hour = TRUE;

  // Minutes
  switch (rtcwake_args -> minutes)
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

  // Date (as a valid DD/MM/YYYY date format)
  timeinfo -> tm_mday = rtcwake_args -> day;
  timeinfo -> tm_mon = rtcwake_args -> month - 1;
  timeinfo -> tm_year = rtcwake_args -> year - 1900;
  timeinfo -> tm_isdst = -1;

  time_check = mktime (timeinfo);

  if (time_check == -1
      || timeinfo -> tm_mday != rtcwake_args -> day
      || timeinfo -> tm_mon != rtcwake_args -> month
      || timeinfo -> tm_year != rtcwake_args -> year)
    timestamp = FALSE;
  else
    timestamp = TRUE;

  // Year (must be this year or at most the next, only)
  get_time_tm (&timeinfo);
  if (rtcwake_args -> year > timeinfo -> tm_year + 1)
    year = FALSE;
  else
    year = TRUE;

  switch (rtcwake_args -> mode)
    {
    case MEM:
    case DISK:
    case OFF:
      mode = TRUE;
      break;

    default:
      mode = FALSE;
    }

  if (hour && minutes && year && timestamp && mode)
    ret = 1;    // valid
  else
    ret = 0;    // invalid

  pthread_mutex_unlock (&rtcwake_args_mutex);

  return ret;
}

static void sync_time (void)
{
  struct tm *timeinfo;

  // Get current time
  get_time_tm (&timeinfo);

  // Calculate how many seconds lacks for the next minute
  int sync_diff = 60 - timeinfo -> tm_sec;

  // If time is already synced, do nothing
  if (sync_diff == 60)
    sync_diff = 0;

  DEBUG_PRINT_TIME (("Syncing time"));
  sleep (sync_diff);
  DEBUG_PRINT_TIME (("Time synced"));
}
