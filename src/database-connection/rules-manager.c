/* rules-manager.c
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

#include "database-connection-utils.h"
#include "rules-manager.h"

int
rule_add (const Rule *rule)
{
  if (utils_validate_rule (rule))
    return EXIT_FAILURE;

  switch (rule->table)
    {
    case TABLE_ON:
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
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

    case TABLE_OFF:
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
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

    case TABLE_LAST:
      return EXIT_FAILURE;

    default:
      return EXIT_FAILURE;
    }

  return utils_run_sql ();
}

int
rule_delete (const uint16_t id,
             const Table table)
{
  if (utils_validate_table (table))
    return EXIT_FAILURE;

  sqlite3_snprintf (SQL_SIZE, utils_get_sql (), "DELETE FROM %s WHERE id = %d;", TABLE[table], id);

  return utils_run_sql ();
}

int
rule_enable_disable (const uint16_t id,
                     const Table table,
                     const bool active)
{
  if (utils_validate_table (table))
    return EXIT_FAILURE;

  sqlite3_snprintf (SQL_SIZE, utils_get_sql (), "UPDATE %s SET active = %d WHERE id = %d;", TABLE[table], active, id);

  return utils_run_sql ();
}

int
rule_edit (const Rule *rule)
{
  if (utils_validate_rule (rule))
    return EXIT_FAILURE;

  switch (rule->table)
    {
    case TABLE_ON:
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
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

    case TABLE_OFF:
      sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
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

    case TABLE_LAST:
    default:
      return EXIT_FAILURE;
    }

  return utils_run_sql ();
}

int
rule_custom_schedule (const uint8_t hour,
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

  // TODO
  /* if (validade_rtcwake_args (&rtcwake_args) == -1) */
  /*   return EXIT_FAILURE; */

  sqlite3_snprintf (SQL_SIZE, utils_get_sql (),
                    "UPDATE custom_schedule "\
                    "SET hour = %d, minutes = %d, "\
                    "day = %d, month = %d, year = %d, "\
                    "mode = %d, use_args = %d "\
                    "WHERE id = 1;",
                    hour, minutes,
                    day, month, year,
                    mode, true);

  int ret = utils_run_sql ();

  // TODO
  /* if (ret == EXIT_SUCCESS) */
  /*   trigger_custom_schedule (); */

  return ret;
}
