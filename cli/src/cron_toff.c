// Checks if there's a turn off rule at the current time; if yes, schedule for the next upcoming turn on rule

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
#include <errno.h>
#include <string.h>

#include "include/gawake.h"
#include "include/get_time.h"
#include "include/wday.h"

// Apdend with ">> path/turn_off.log 2>&1"
#define LOGS_OUTPUT " >> " LOGS "turn_off.log 2>&1"

int main (void) {
  int rc, now, db_time = 1, id_match = -1, alloc = 230, cmd_stat = 0, gawake_stat = 0;

  struct tm *timeinfo;                // Default structure, see documentation
  struct sqlite3_stmt *stmt;

  sqlite3 *db;

  char time[7];
  const char *DB_TIMES[] = {"utc", "localtime"};
  const char *DAYS[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};     // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
  char query[alloc], rtcwake_cmd[alloc], buff[7], date[9], mode[8], options[alloc], cmd[CMD_LEN];

  // GET THE CURRENT TIME
  get_time(&timeinfo);                                                    // hour, minutes and seconds as integer members
  snprintf(buff, 7, "%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min); // Concatenate: HHMM as a string
  now = atoi(buff);                                                       // HHMM as an integer, leading zeros doesn't matter

  fprintf(stdout, "\n%s", asctime(timeinfo));

  // OPEN DATABASE
  rc = sqlite3_open_v2(PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc  != SQLITE_OK) {
    fprintf(stderr, "Couldn't open database: %s\n", sqlite3_errmsg(db));
    return EXIT_FAILURE;
  }

  // GET THE DATABASE CONFIG
  if (sqlite3_prepare_v2(db, "SELECT localtime, def_mode, options, commands, status FROM config WHERE id = 1;", -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "ERROR: Failed getting config information\n");
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    db_time = sqlite3_column_int(stmt, 0);                            // from the database: 0 = utc, 1 = localtime
    snprintf(mode, 8, "%s", sqlite3_column_text(stmt, 1));            // default mode, since this schedule doesn't come from a turn off rule
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

  // CHECK IF THERE IS A TURN OFF RULE FOR NOW
  int match = 0;
  snprintf(query, alloc, "SELECT strftime('%%H%%M', time), command FROM rules_turnoff WHERE %s = 1 ORDER BY time(time) ASC;",
           DAYS[timeinfo -> tm_wday]);
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: Failed scheduling for today\n");
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  // Get all rules today, ordered by time; the first rule that has a bigger time than now is a valid
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int ruletime = sqlite3_column_int(stmt, 0);
    if (now == ruletime) {
      snprintf(cmd, CMD_LEN, "%s", sqlite3_column_text(stmt, 1));
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
  if (match != 1) {
    fprintf(stdout, "Any rule matched\n");
    return EXIT_SUCCESS;
  }

  // TRY TO SCHEDULE FOR TODAY
  // Since turn on rules have time in format HHMMSS, to make comparations, reformat it
  snprintf(buff, 7, "%02d%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec); // Concatenate: HHMMSS as a string
  now = atoi(buff);                                                                               // HHMMSS as an integer, leading zeros doesn't matter

  fprintf(stdout, "Rule matched: trying to schedule for today\n");
  // Create an SQL statement to get today's active rules time; tm_wday = number of the week
  snprintf(query, alloc, "SELECT id, strftime('%%H%%M%%S', time), strftime('%%Y%%m%%d', 'now', '%s') FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC;", DB_TIMES[db_time], DAYS[timeinfo -> tm_wday]);
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: Failed scheduling for today\n");
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  // Get all rules today, ordered by time; the first rule that has a bigger time than now is a valid
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int id =        sqlite3_column_int(stmt, 0);
    int ruletime =  sqlite3_column_int(stmt, 1);
    if (now < ruletime) {
      id_match = id;
      snprintf(date, 9, "%s", sqlite3_column_text(stmt, 2));        // YYYYMMDD
      snprintf(time, 7, "%s", sqlite3_column_text(stmt, 1));        // HHMMSS
      break;
    }
  }
  if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
    fprintf(stderr, "ERROR (failed scheduling for today): %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // IF IT WASN'T POSSIBLE TO SCHEDULE FOR TODAY, TRY ON THE NEXT DAYS
  if (id_match < 0) {
    fprintf(stdout, "Any time matched. Trying to schedule for tomorrow or later\n");
    // search for a matching rule within a week
    for (int i = 1; i <= 7; i++) {
      int wday_num = wday(timeinfo -> tm_wday + i);
      if (wday_num == -1) {
        fprintf(stderr, "ERROR: Failed scheduling for after (on wday function)\n");
        sqlite3_close(db);
        return EXIT_FAILURE;
      }
      // The first rule after today is a valid match;
      // also calculate the day: now + number of day until the matching rule, represented by the index i of the loop
      snprintf(query, alloc, "SELECT id, strftime('%%Y%%m%%d', 'now', '%s', '+%d day'), strftime('%%H%%M%%S', time) "\
               "FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC LIMIT 1;",
               DB_TIMES[db_time], i, DAYS[wday_num]);
      rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        fprintf(stderr, "ERROR: Failed scheduling for after\n");
        return EXIT_FAILURE;
      }
      while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        id_match = sqlite3_column_int(stmt, 0);
        snprintf(date, 9, "%s", sqlite3_column_text(stmt, 1));        // YYYYMMDD
        snprintf(time, 7, "%s", sqlite3_column_text(stmt, 2));        // HHMMSS
      }
      if (rc != SQLITE_DONE) {
        fprintf(stderr, "ERROR (failed scheduling for after): %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return EXIT_FAILURE;
      }
      sqlite3_finalize(stmt);
      if (id_match >= 0) {
        break;
      }
    }
  }

  // IF ANY RULE WAS FOUND, RETURN
  if (id_match < 0) {
    fprintf(stdout, "WARNING: Any turn on rule found. Schedule failed!\n");
    return EXIT_SUCCESS;
  }
  // ELSE, SCHEDULE
  int stat = 0;
  fprintf(stdout, "Match on turn on rule with ID [%d]\n", id_match);
  if (cmd_stat && cmd[0] != '\0') { // If the commands are enabled, and if there's a command on the databse
    fprintf(stdout, "Running command: %s\n", cmd);
    stat = system(cmd);
    if (stat != 0)
      fprintf(stderr, "[!] >>>>>>>>> Command(set by user) exited with error\n");
  }
  snprintf(rtcwake_cmd, alloc, "sudo rtcwake --date %s%s %s -m %s", date, time, options, mode);
  fprintf(stdout, "Running rtcwake: %s\n", rtcwake_cmd);
  strcat(rtcwake_cmd, LOGS_OUTPUT);    // Sending rtcwake output to the log
  stat = system(rtcwake_cmd);
  if (stat != 0)
    fprintf(stderr, "[!] >>>>>>>>> rtcwake failed scheduling\n");
  return EXIT_SUCCESS;
}
