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
#include <time.h>

#include "../utils/debugger.h"
#include "database-connection-utils.h"

static sqlite3 *db = NULL;
static char sql[SQL_SIZE];

// Get the system time (now) as time_t
static
int get_time (time_t *time_now)
{
  time (time_now);
  if (*time_now ==  (time_t) -1)
    {
      fprintf (stderr, "ERROR: failed while getting time\n");
      return EXIT_FAILURE;
    }
  else
    return EXIT_SUCCESS;
}

int
utils_validate_rule (const Rule *rule)
{
  // name can't be bigger than the max value
  bool name = !(strlen (rule->name) >= RULE_NAME_LENGTH);     // (>=) do not include null terminator

  // hour [00,23]
  bool hour = rule->hour <= 23;     // Due to the data type, it is always >= 0

  // minutes [0, 59]
  bool minutes = rule->minutes <= 59;

  // mode according to enum predefined values
  bool mode = (rule->mode >= 0 && rule->mode <= MODE_OFF);

  bool table = (rule->table == TABLE_ON || rule->table == TABLE_OFF);

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
utils_validate_table (const Table table)
{
  if (table == TABLE_ON || table == TABLE_OFF)
    return EXIT_SUCCESS;

  fprintf (stderr, "Invalid table\n\n");
  return EXIT_FAILURE;
}

int
utils_run_sql (void)
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

char *
utils_get_sql (void)
{
  return sql;
}

sqlite3 * utils_get_pdb (void)
{
  return db;
}

sqlite3 ** utils_get_ppdb (void)
{
  return &db;
}

int
utils_validade_rtcwake_args (RtcwakeArgs *rtcwake_args)
{
  DEBUG_PRINT (("Validating rtcwake_args..."));

  bool hour, minutes, date, year, mode;
  hour = minutes = date = year  = mode = false;
  int ret;
  struct tm *timeinfo;

  // Hour
  if (rtcwake_args->hour >= 0 && rtcwake_args->hour <= 23)
    hour = true;

  // Minutes
  if (rtcwake_args->minutes >= 0 && rtcwake_args->minutes <= 59)
    minutes = true;

  // Date
  struct tm input = {
    .tm_mday = rtcwake_args->day,
    .tm_mon = rtcwake_args->month - 1,
    .tm_year = rtcwake_args->year - 1900,
  };
  time_t generated_time = mktime (&input);
  timeinfo = localtime (&generated_time);
  if (generated_time == -1
      || rtcwake_args->day != timeinfo->tm_mday
      || rtcwake_args->month != timeinfo->tm_mon + 1
      || rtcwake_args->year != timeinfo->tm_year + 1900)
    date = false;
  else
    date = true;

  // Year (must be this year or at most the next, only)
  get_time_tm (&timeinfo);
  if (rtcwake_args->year > (timeinfo->tm_year + 1900 + 1))
    year = false;
  else
    year = true;

  switch (rtcwake_args->mode)
    {
    case MODE_MEM:
    case MODE_DISK:
    case MODE_OFF:
    case MODE_NO:
      mode = true;
      break;

    case MODE_LAST:
    default:
      mode = false;
    }

  if (hour && minutes && date && year && mode)
    ret = 1;    // valid
  else
    ret = -1;   // invalid

  DEBUG_PRINT (("RtcwakeArgs validation:\n"\
                "\tHour: %d\n\tMinutes: %d\n\tDate: %d\n\tYear: %d\n"\
                "\tMode: %d\n\tthis_year: %d\n\t--> Passed: %d",
                hour, minutes, date, year, mode, timeinfo->tm_year + 1900, ret));

  return ret;
}
