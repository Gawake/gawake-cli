// Checks if there's a turn on rule at the current time; if yes, run the command

/*
 * Gawake. A Linux software to make your PC wake up on a scheduled time. It makes the rtcwake command easier.
 *
 * Copyright (C) 2021 - 2023, Kelvin Ribeiro Novais
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, considering ONLY the version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "include/gawake.h"
#include "include/get_time.h"

int main (void) {
  int rc, now, alloc = 192, cmd_stat = 0, gawake_stat = 0, boot_time = 0;

  struct tm *timeinfo;        // Default structure, see documentation
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  const char *DAYS[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};      // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
  char query[alloc], buff[7], cmd[CMD_LEN];

  // GET THE CURRENT TIME
  get_time(&timeinfo);                                                                            // hour, minutes and seconds as integer members
  snprintf(buff, 7, "%02d%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec); // Concatenate: HHMMSS as a string
  now = atoi(buff);                                                                               // HHMMSS as an integer, leading zeros doesn't matter

  fprintf(stdout, "\n%s", asctime(timeinfo));

  // OPEN DATABASE
  rc = sqlite3_open_v2(PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc  != SQLITE_OK) {
    fprintf(stderr, "Couldn't open database: %s\n", sqlite3_errmsg(db));
    return EXIT_FAILURE;
  }

  // GET THE DATABASE CONFIG
  if (sqlite3_prepare_v2(db, "SELECT commands, boot_time, status FROM config WHERE id = 1;", -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "ERROR: Failed getting config information\n");
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    cmd_stat =    sqlite3_column_int(stmt, 0);
    boot_time =   sqlite3_column_int(stmt, 1);
    gawake_stat = sqlite3_column_int(stmt, 2);
  }
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR (failed getting config information): %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // Exit if commands or gawake are disabled
  if (cmd_stat == 0 || gawake_stat == 0) {
    fprintf(stdout, "Commands or Gawake are disabled. Exiting...\n");
    return EXIT_SUCCESS;
  }

  // CHECK IF THERE IS A TURN OFF RULE FOR NOW
  int match = 0, id_match = -1;
  snprintf(query, alloc, "SELECT strftime('%%H%%M%%S', time), command, id FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC;",
           DAYS[timeinfo -> tm_wday]);
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: Failed searching for commands\n");
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  // Get all rules today, ordered by time
  // The first rule that has a time between now + the boot time (tolerance), is valid
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int ruletime = sqlite3_column_int(stmt, 0);
    if (now >= ruletime && now <= ruletime + boot_time) {
      snprintf(cmd, CMD_LEN, "%s", sqlite3_column_text(stmt, 1));
      id_match = sqlite3_column_int(stmt, 2);
      match = 1;
      break;
    }
  }
  if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
    fprintf(stderr, "ERROR (failed scheduling for today): %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // If there was no match, or the command is empty, exit
  if (match != 1 || cmd[0] == '\0') {
    fprintf(stdout, "Any rule matched\n");
    return EXIT_SUCCESS;
  } else {
    fprintf(stdout, "Match on turn on rule with ID [%d]\n", id_match);
    fprintf(stdout, "Running command: %s\n", cmd);
    int stat = system(cmd);
    if (stat != 0)
      fprintf(stderr, "Command exited with error\n");
  }

  return EXIT_SUCCESS;
}
