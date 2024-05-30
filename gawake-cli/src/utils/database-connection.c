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

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database-connection.h"

#include "validate-rtcwake-args.h"
#include "debugger.h"
#include "colors.h"
#include "dbus-client.h"

static sqlite3 *db = NULL;
static char *sql = 0;

// This function connect to the database
// Should be called once
int connect_database (void)
{
  // Allocate memory
  sql = (char *) malloc (ALLOC);
  if (sql == NULL)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to allocate memory\n"));
      return EXIT_FAILURE;
    }

  // Open the SQLite database
  int rc = sqlite3_open_v2 (DB_PATH, &db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("Can't open database: %s\n"), sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  else
    {
      char *err_msg = 0;

      const char UPDATE_VERSION[] = "UPDATE config SET version = '" VERSION "';";
      rc = sqlite3_exec (db, UPDATE_VERSION, NULL, 0, &err_msg);

      sqlite3_free (err_msg);
    }

  connect_dbus_client ();

  return EXIT_SUCCESS;
}

void close_database (void)
{
  sqlite3_close (db);
  free (sql);

  close_dbus_client ();
}

static int validate_rule (const Rule *rule)
{
  // name can't be bigger than the max value
  bool name = !(strlen (rule->name) >= RULE_NAME_LENGTH);     // (>=) do not include null terminator

  // hour [00,23]
  bool hour = rule->hour <= 23;     // Due to the data type, it is always >= 0

  // minutes according to enum predefined values
  bool minutes = rule->minutes <= 59;

  // mode according to enum predefined values
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

static int validate_table (const Table table)
{
  if (table == T_ON || table == T_OFF)
    return EXIT_SUCCESS;

  fprintf (stderr, "Invalid table\n\n");
  return EXIT_FAILURE;
}

static int run_sql (void)
{
  int rc;
  char *err_msg = 0;

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
  if (rc != SQLITE_OK)
    fprintf (stderr, "Failed to run SQL: %s\n", sqlite3_errmsg(db));

  sqlite3_free (err_msg);

  if (rc == SQLITE_OK)
    {
      trigger_update_database ();
      return EXIT_SUCCESS;
    }
  else
    return EXIT_FAILURE;
}

int add_rule (const Rule *rule)
{
  if (validate_rule (rule))
    return EXIT_FAILURE;

  switch (rule->table)
    {
    case T_ON:
      sqlite3_snprintf (ALLOC, sql,
                        "INSERT INTO rules_turnon "\
                        "(rule_name, rule_time, sun, mon, tue, wed, thu, fri, sat, active) "\
                        "VALUES ('%s', '%02d:%02u:00', %d, %d, %d, %d, %d, %d, %d, %d);",
                        rule->name,
                        rule->hour,
                        rule->minutes,
                        rule->days[0], rule->days[1], rule->days[2], rule->days[3],
                        rule->days[4], rule->days[5], rule->days[6],
                        rule->active);
      break;
    case T_OFF:
      sqlite3_snprintf (ALLOC, sql,
                        "INSERT INTO rules_turnoff "\
                        "(rule_name, rule_time, sun, mon, tue, wed, thu, fri, sat, active, mode) "\
                        "VALUES ('%s', '%02d:%02u:00', %d, %d, %d, %d, %d, %d, %d, %d, %u);",
                        rule->name,
                        rule->hour,
                        rule->minutes,
                        rule->days[0], rule->days[1], rule->days[2], rule->days[3],
                        rule->days[4], rule->days[5], rule->days[6],
                        rule->active,
                        rule->mode);
      break;
    default:
      return EXIT_FAILURE;
    }

  return run_sql ();
}

int edit_rule (const Rule *rule)
{
  if (validate_rule (rule))
    return EXIT_FAILURE;

  switch (rule->table)
    {
    case T_ON:
      sqlite3_snprintf (ALLOC, sql,
                        "UPDATE rules_turnon SET "\
                        "rule_name = '%s', rule_time = '%02d:%02d:00', "\
                        "sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                        "active = %d WHERE id = %d;",
                        rule->name,
                        rule->hour,
                        rule->minutes,
                        rule->days[0], rule->days[1], rule->days[2], rule->days[3],
                        rule->days[4], rule->days[5], rule->days[6],
                        rule->active,
                        rule->id);
      break;
    case T_OFF:
      sqlite3_snprintf (ALLOC, sql,
                        "UPDATE rules_turnoff SET "\
                        "rule_name = '%s', rule_time = '%02d:%02d:00', "\
                        "sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                        "active = %d, mode = %d WHERE id = %d;",
                        rule->name,
                        rule->hour,
                        rule->minutes,
                        rule->days[0], rule->days[1], rule->days[2], rule->days[3],
                        rule->days[4], rule->days[5], rule->days[6],
                        rule->active,
                        rule->mode,
                        rule->id);
      break;
    default:
      return EXIT_FAILURE;
    }

  return run_sql ();
}

int delete_rule (const uint16_t id, const Table table)
{
  if (validate_table (table))
    return EXIT_FAILURE;

  sqlite3_snprintf (ALLOC, sql, "DELETE FROM %s WHERE id = %d;", TABLE[table], id);

  return run_sql ();
}

int enable_disable_rule (const uint16_t id, const Table table, const bool active)
{
  if (validate_table (table))
    return EXIT_FAILURE;

  sqlite3_snprintf (ALLOC, sql, "UPDATE %s SET active = %d WHERE id = %d;", TABLE[table], active, id);

  return run_sql ();
}

int query_rule (const uint16_t id, const Table table, Rule *rule)
{
  if (validate_table (table))
    return EXIT_FAILURE;

  // Database related variables
  int rc;
  struct sqlite3_stmt *stmt;

  // Generate SQL
  sqlite3_snprintf (ALLOC, sql, "SELECT * FROM %s WHERE id = %d LIMIT 1;", TABLE[table], id);

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  // Verify ID (also prepare statement)
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      fprintf (stderr, "Invalid ID\n\n");
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  // Query data
  /* ATTENTION columns numbers:
   *    0     1             2       3       (...)       9       10          11
   *    id    rule_name     time    sun     (...)       sat     active      mode
   *                                                                        ^~~~
   *                                                                        |
   *                                                  only for turn off rules
   */
  // Temporary variables to receive the minutes and pass to the structure;
  int hour, minutes;
  char timestamp[9]; // HH:MM:SS'\0' = 9 characters
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // ID
      rule->id = (uint16_t) sqlite3_column_int (stmt, 0);
      // NAME
      snprintf (rule->name, RULE_NAME_LENGTH, "%s", sqlite3_column_text (stmt, 1));

      // MINUTES AND HOUR
      sqlite3_snprintf (9, timestamp, "%s", sqlite3_column_text (stmt, 2));
      sscanf (timestamp, "%02d:%02d", &hour, &minutes);
      rule->hour = (uint8_t) hour;
      rule->minutes = minutes;

      // DAYS
      for (int i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [3,9]
          rule->days[i] = (bool) sqlite3_column_int (stmt, (i+3));
        }

      // ACTIVE
      rule->active = (bool) sqlite3_column_int (stmt, 10); // active

      // MODE (for turn on rules it isn't used):
      rule->mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 11) : 0);

      // TABLE
      rule->table = (Table) table;
    }

  if (rc != SQLITE_DONE)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg (db));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize (stmt);

  return EXIT_SUCCESS;
}

int query_rules (const Table table, Rule **rules, uint16_t *rowcount)
{
  if (validate_table (table))
    return EXIT_FAILURE;

  // Database related variables
  int rc;
  struct sqlite3_stmt *stmt;

  // Count the number of rows
  sqlite3_snprintf (ALLOC, sql, "SELECT COUNT(*) FROM %s;", TABLE[table]);
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to query row count\n"));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }
  *rowcount = sqlite3_column_int (stmt, 0);

  DEBUG_PRINT (("Row count: %d", *rowcount));

  // Allocate structure array
  // https://www.youtube.com/watch?v=lq8tJS3g6tY
  *rules = malloc (*rowcount * sizeof (**rules));
  if (*rules == NULL)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to allocate memory\n"));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  // Generate SQL
  // SELECT length(<table>.rule_name), * FROM <table>;
  sqlite3_snprintf (ALLOC, sql, "SELECT length(%s.rule_name), * FROM %s;", TABLE[table], TABLE[table]);

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  // Prepare statement
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to query rule\n"));
      free (*rules);
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  int counter = 0;
  // Query data
  /* ATTENTION columns numbers:
   *    0                     1     2             3       4       (...)       10      11        12
   *    rule_name length      id    rule_name     time    sun     (...)       sat     active    mode
   *                                                                                            ^~~~
   *                                                                                            |
   *                                                                      only for turn off rules
   */
  // Temporary variables to receive the hour and minutes, and then pass to the structure
  int hour, minutes;
  char timestamp[9]; // HH:MM:SS'\0' = 9 characters
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // If the loop tries to assign on non allocated space, leave
      if (counter > *rowcount)
        break;

      // ID
      (*rules)[counter].id = (uint16_t) sqlite3_column_int (stmt, 1);

      // NAME
      // Allocate memory for the name
      int size = sqlite3_column_int (stmt, 0);
      (*rules)[counter].name = (char *) malloc (size + 1);
      if ((*rules)[counter].name == NULL)
        {
          DEBUG_PRINT_CONTEX;
          // TODO avoid memory leaking: free names allocated up to here and then *rules
          fprintf (stderr, RED ("ERROR: Failed to allocate memory\n"));
          return EXIT_FAILURE;
        }

      // Assign name
      snprintf ((*rules)[counter].name,         // string pointer
                  size + 1,                       // size
                  "%s",                           // format
                  sqlite3_column_text (stmt, 2)   // arguments
                  );

      // MINUTES AND HOUR
      sqlite3_snprintf (9, timestamp, "%s", sqlite3_column_text (stmt, 3));
      sscanf (timestamp, "%02d:%02d", &hour, &minutes);
      (*rules)[counter].hour =  (uint8_t) hour;
      (*rules)[counter].minutes = (uint8_t) minutes;

      // DAYS
      for (int i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [4,10]
          (*rules)[counter].days[i] = (bool) sqlite3_column_int (stmt, (i+4));
        }

      // ACTIVE
      (*rules)[counter].active = (bool) sqlite3_column_int (stmt, 11);

      // MODE (for turn on rules it isn't used, assigning 0):
      (*rules)[counter].mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 12) : 0);

      // TABLE
      (*rules)[counter].table = (Table) table;

      counter++;
    }

  if (rc != SQLITE_DONE)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR (failed to query rule): %s\n"), sqlite3_errmsg (db));
      free (*rules);
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize(stmt);

  return EXIT_SUCCESS;
}

int custom_schedule (const uint8_t hour,
                          const uint8_t minutes,
                          const uint8_t day,
                          const uint8_t month,
                          const uint16_t year,
                          const uint8_t mode)
{
  RtcwakeArgs rtcwake_args = {
    .hour = hour,
    .minutes = minutes,
    .day = day,
    .month = month,
    .year = year,
    .mode = (Mode) mode,
  };

  if (validade_rtcwake_args (&rtcwake_args) == -1)
    return EXIT_FAILURE;

  sqlite3_snprintf (ALLOC, sql,
                    "UPDATE custom_schedule "\
                    "SET hour = %d, minutes = %d, "\
                    "day = %d, month = %d, year = %d, "\
                    "mode = %d, use_args = %d "\
                    "WHERE id = 1;",
                    hour, minutes,
                    day, month, year,
                    mode, true);

  int ret = run_sql ();

  if (ret == EXIT_SUCCESS)
    trigger_custom_schedule ();

  return ret;
}

int set_localtime (bool use_localtime)
{
  sqlite3_snprintf (ALLOC, sql,
                    "UPDATE config "\
                    "SET localtime = %d "\
                    "WHERE id = 1;",
                    use_localtime);

  return run_sql ();
}

int set_default_mode (Mode default_mode)
{
  // Validate mode
  if (default_mode >= MEM && default_mode <= OFF)
    {
      sqlite3_snprintf (ALLOC, sql,
                        "UPDATE config "\
                        "SET default_mode = %d "\
                        "WHERE id = 1;",
                        default_mode);

      return run_sql ();
    }
  else
    return EXIT_FAILURE;
}

int set_notification_time (int notification_time)
{
  if (notification_time >= 0 && notification_time <= MAX_NOTIFICATION_TIME)
    {
      sqlite3_snprintf (ALLOC, sql,
                        "UPDATE config "\
                        "SET notification_time = %d "\
                        "WHERE id = 1;",
                        notification_time);

      return run_sql ();
    }
  else
    return EXIT_FAILURE;
}

int set_shutdown_fail (bool shutdown_fail)
{
  sqlite3_snprintf (ALLOC, sql,
                    "UPDATE config "\
                    "SET shutdown_fail = %d "\
                    "WHERE id = 1;",
                    shutdown_fail);

  return run_sql ();
}

int get_config (Config *config)
{
  // Database related variables
  int rc;
  struct sqlite3_stmt *stmt;

  // Generate SQL
  sqlite3_snprintf (ALLOC, sql,
                    "SELECT * FROM config "\
                    "WHERE id = 1;");

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to query rule\n"));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // Note: column 0 is the id
      // Note: column 1 is the cli_version

      // Use localtime
      config->use_localtime = (bool) sqlite3_column_int (stmt, 2);

      // Default mode
      config->default_mode = (Mode) sqlite3_column_int (stmt, 3);

      // Notification time
      config->notification_time = sqlite3_column_int (stmt, 4);

      // Shutdown if fails
      config->shutdown_fail = (bool) sqlite3_column_int (stmt, 5);
    }

  if (rc != SQLITE_DONE)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR (failed to query rule): %s\n"), sqlite3_errmsg (db));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize(stmt);

  return EXIT_SUCCESS;
}

void schedule (void)
{
  trigger_schedule ();
}
