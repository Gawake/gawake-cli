/* rules-reader.c
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
#include "../utils/debugger.h"
#include "../utils/colors.h"
#include "rules-reader.h"

int
rule_get_single (const uint16_t id,
                 const Table table,
                 Rule *rule)
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
  if (sqlite3_prepare_v2 (get_pdb (), sql, -1, &stmt, NULL) == SQLITE_OK
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
      fprintf (stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg (get_pdb ()));
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize (stmt);

  return EXIT_SUCCESS;
}

int
rule_get_all (const Table table,
              Rule **rules,
              uint16_t *rowcount)
{
  int counter = 0;
  // Database related variables
  int rc;
  struct sqlite3_stmt *stmt;
  // Temporary variables to receive the hour and minutes, and then pass to the structure
  int hour, minutes;
  char timestamp[9]; // HH:MM:SS'\0' = 9 characters


  if (validate_table (table))
    return EXIT_FAILURE;

  // Count the number of rows
  sqlite3_snprintf (ALLOC, sql, "SELECT COUNT(*) FROM %s;", TABLE[table]);
  if (sqlite3_prepare_v2 (get_pdb (), sql, -1, &stmt, NULL) == SQLITE_OK
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
  // TODO should "sizeof (**rules)" be "sizeof (**Rules)"
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
  sqlite3_snprintf (ALLOC, sql,
                    "SELECT length(%s.rule_name), * FROM %s;",
                    TABLE[table], TABLE[table]);

  DEBUG_PRINT (("Generated SQL:\n\t%s", sql));

  // Prepare statement
  if (sqlite3_prepare_v2 (get_pdb (), sql, -1, &stmt, NULL) != SQLITE_OK)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to query rule\n"));
      free (*rules);
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  // Query data
  /* ATTENTION columns numbers:
   *    0                     1     2             3       4       (...)       10      11        12
   *    rule_name length      id    rule_name     time    sun     (...)       sat     active    mode
   *                                                                                            ^~~~
   *                                                                                            |
   *                                                                      only for turn off rules
   */
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
          // Avoid memory leaking: free names allocated up to here
          for (int i = 0; i < counter; i++)
            free ((*rules)[i].name);

          fprintf (stderr, RED ("ERROR: Failed to allocate memory\n"));
          return EXIT_FAILURE;
        }

      // Assign name
      snprintf ((*rules)[counter].name,           // string pointer
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
      fprintf (stderr, RED ("ERROR (failed to query rule): %s\n"), sqlite3_errmsg (get_pdb ()));
      free (*rules);
      sqlite3_finalize (stmt);
      return EXIT_FAILURE;
    }

  sqlite3_finalize(stmt);

  return EXIT_SUCCESS;
}
