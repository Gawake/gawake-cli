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

#include "database-connection.h"

#include "../utils/validate-rtcwake-args.h"
#include "../utils/debugger.h"

static sqlite3 *db = NULL;

// This function connect to the database
// Should be called once
gint connect_database (void)
{
  // Open the SQLite database
  gint rc = sqlite3_open_v2 (DB_NAME, &db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      g_fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  else
    {
      gchar *err_msg = 0;

      const gchar UPTDATE_VERSION[] = "UPDATE config SET version = '" VERSION "';";
      rc = sqlite3_exec (db, UPTDATE_VERSION, NULL, 0, &err_msg);
      g_fprintf (stdout, "Database opened successfully\n");

      sqlite3_free (err_msg);
    }

  return EXIT_SUCCESS;
}

void close_database (void)
{
  g_fprintf (stdout, "Closing database\n");

  sqlite3_close (db);
}

static gint validate_rule (const gRule *rule)
{
  // name can't be bigger than the max value
  gboolean name = !(strlen (rule->name) >= RULE_NAME_LENGTH);     // (>=) do not include null terminator

  // hour [00,23]
  gboolean hour = rule->hour <= 23;     // Due to the data type, it is always >= 0

  // minutes according to enum predefined values
  gboolean minutes = rule->minutes <= 59;

  // mode according to enum predefined values
  gboolean mode = (rule->mode >= 0 && rule->mode <= OFF);

  gboolean table = (rule->table == T_ON || rule->table == T_OFF);

  DEBUG_PRINT (("VALIDATION:\nName: %d\n\tName length: %lu\n"\
                "Hour: %d\nMinutes: %d\nMode: %d\nTable: %d",
                name, strlen (rule->name), hour, minutes, mode, table));

  if (name && hour && minutes && mode && table)
    return EXIT_SUCCESS;
  else
    {
      g_fprintf (stderr, "Invalid rule values\n\n");
      return EXIT_FAILURE;
    }
}

static gint validate_table (const Table table)
{
  if (table == T_ON || table == T_OFF)
    return EXIT_SUCCESS;

  g_fprintf (stderr, "Invalid table\n\n");
  return EXIT_FAILURE;
}

static gboolean run_sql (const gchar *sql)
{
  gint rc;
  gchar *err_msg = 0;

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
  if (rc != SQLITE_OK)
    g_fprintf (stderr, "Failed to run SQL: %s\n", sqlite3_errmsg(db));

  sqlite3_free (err_msg);

  if (rc == SQLITE_OK)
    return TRUE;
  else
    return FALSE;
}

gboolean add_rule (const gRule *rule)
{
  if (validate_rule (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

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
      g_free (sql);
      return FALSE;
    }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean edit_rule (const gRule *rule)
{
  if (validate_rule (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

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
      g_free (sql);
      return FALSE;
    }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean delete_rule (const guint16 id, const Table table)
{
  if (validate_table (table))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  sqlite3_snprintf (ALLOC, sql, "DELETE FROM %s WHERE id = %d;", TABLE[table], id);

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean enable_disable_rule (const guint16 id, const Table table, const gboolean active)
{
  if (validate_table (table))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      g_fprintf (stderr, "Could'n allocate memory\n\n");
      return FALSE;
    }

  sqlite3_snprintf (ALLOC, sql, "UPDATE %s SET active = %d WHERE id = %d;", TABLE[table], active, id);

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean query_rule (const guint16 id, const Table table, gRule *rule)
{
  if (validate_table (table))
    return FALSE;

  // Database related variables
  gint rc;
  struct sqlite3_stmt *stmt;
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Generate SQL
  sqlite3_snprintf (ALLOC, sql, "SELECT * FROM %s WHERE id = %d LIMIT 1;", TABLE[table], id);

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  // Verify ID (also prepare statement)
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      g_fprintf (stderr, "Invalid ID\n\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
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
  gint hour, minutes;
  gchar timestamp[9]; // HH:MM:SS'\0' = 9 characters
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // ID
      rule->id = (guint16) sqlite3_column_int (stmt, 0);
      // NAME
      g_snprintf (rule->name, RULE_NAME_LENGTH, "%s", sqlite3_column_text (stmt, 1));

      // MINUTES AND HOUR
      sqlite3_snprintf (9, timestamp, "%s", sqlite3_column_text (stmt, 2));
      sscanf (timestamp, "%02d:%02d", &hour, &minutes);
      rule->hour = (guint8) hour;
      rule->minutes = minutes;

      // DAYS
      for (gint i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [3,9]
          rule->days[i] = (gboolean) sqlite3_column_int (stmt, (i+3));
        }

      // ACTIVE
      rule->active = (gboolean) sqlite3_column_int (stmt, 10); // active

      // MODE (for turn on rules it isn't used):
      rule->mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 11) : 0);

      // TABLE
      rule->table = (Table) table;
    }

  if (rc != SQLITE_DONE)
    {
      g_fprintf (stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg (db));
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  sqlite3_finalize (stmt);
  g_free (sql);

  return TRUE;
}

gboolean query_rules (const Table table, gRule **rules, guint16 *rowcount)
{
  if (validate_table (table))
    return FALSE;

  // Database related variables
  gint rc;
  struct sqlite3_stmt *stmt;
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Count the number of rows
  sqlite3_snprintf (ALLOC, sql, "SELECT COUNT(*) FROM %s;", TABLE[table]);
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      g_fprintf (stderr, "ERROR: Failed to query row count\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }
  *rowcount = sqlite3_column_int (stmt, 0);

  DEBUG_PRINT (("Row count: %d", *rowcount));

  // Allocate structure array
  *rules = malloc (*rowcount * sizeof (**rules));
  if (*rules == NULL)
    {
      g_fprintf (stderr, "ERROR: Failed to allocate memory\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Generate SQL
  // SELECT length(<table>.rule_name), * FROM <table>;
  sqlite3_snprintf (ALLOC, sql, "SELECT length(%s.rule_name), * FROM %s;", TABLE[table], TABLE[table]);

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  // Prepare statement
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
      g_fprintf (stderr, "ERROR: Failed to query rule\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  gint counter = 0;
  // Query data
  /* ATTENTION columns numbers:
   *    0                     1     2             3       4       (...)       10      11        12
   *    rule_name length      id    rule_name     time    sun     (...)       sat     active    mode
   *                                                                                            ^~~~
   *                                                                                            |
   *                                                                      only for turn off rules
   */
  // Temporary variables to receive the hour and minutes, and then pass to the structure
  gint hour, minutes;
  gchar timestamp[9]; // HH:MM:SS'\0' = 9 characters
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // If the loop tries to assign on non allocated space, leave
      if (counter > *rowcount)
        break;

      // ID
      (*rules)[counter].id = (guint16) sqlite3_column_int (stmt, 1);

      // NAME
      // Allocate memory for the name
      gint size = sqlite3_column_int (stmt, 0);
      (*rules)[counter].name = (gchar *) g_malloc (size + 1);
      if ((*rules)[counter].name == NULL)
        {
          g_fprintf (stderr, "ERROR: Failed to allocate memory\n");
          return FALSE;
        }

      // Assign name
      g_snprintf ((*rules)[counter].name,         // string pointer
                  size + 1,                       // size
                  "%s",                           // format
                  sqlite3_column_text (stmt, 2)   // arguments
                  );

      // MINUTES AND HOUR
      sqlite3_snprintf (9, timestamp, "%s", sqlite3_column_text (stmt, 3));
      sscanf (timestamp, "%02d:%02d", &hour, &minutes);
      (*rules)[counter].hour =  (guint8) hour;
      (*rules)[counter].minutes = (guint8) minutes;

      // DAYS
      for (gint i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [4,10]
          (*rules)[counter].days[i] = (gboolean) sqlite3_column_int (stmt, (i+4));
        }

      // ACTIVE
      (*rules)[counter].active = (gboolean) sqlite3_column_int (stmt, 11);

      // MODE (for turn on rules it isn't used, assigning 0):
      (*rules)[counter].mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 12) : 0);

      // TABLE
      (*rules)[counter].table = (Table) table;

      counter++;
    }

  if (rc != SQLITE_DONE)
    {
      g_fprintf (stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg (db));
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  sqlite3_finalize(stmt);
  g_free (sql);

  return TRUE;
}

gboolean custom_schedule (const guint8 hour,
                          const guint8 minutes,
                          const guint8 day,
                          const guint8 month,
                          const guint16 year,
                          const guint8 mode)
{
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  RtcwakeArgs rtcwake_args = {
    .hour = hour,
    .minutes = minutes,
    .day = day,
    .month = month,
    .year = year,
    .mode = (Mode) mode,
  };

  if (validade_rtcwake_args (&rtcwake_args) == -1)
    return FALSE;

  sqlite3_snprintf (ALLOC, sql,
                    "UPDATE custom_schedule "\
                    "SET hour = %d, minutes = %d, "\
                    "day = %d, month = %d, year = %d, "\
                    "mode = %d "\
                    "WHERE id = 1;",
                    hour, minutes,
                    day, month, year,
                    mode);

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}
