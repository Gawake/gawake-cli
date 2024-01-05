// Checks if the Gawake directory and database exist, if not, create them. Requires root permissions.

/* database-checker.c
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

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "include/gawake.h"
#include "include/issue.h"

int main(void) {
  int rc, fd;
  sqlite3 *db;
  const char *SQL = "CREATE TABLE IF NOT EXISTS rules_turnon ("\
                      "id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
                      "rule_name   TEXT NOT NULL,"\
                      "time        TEXT NOT NULL,"\
                      "sun         INTEGER NOT NULL,"\
                      "mon         INTEGER NOT NULL,"\
                      "tue         INTEGER NOT NULL,"\
                      "wed         INTEGER NOT NULL,"\
                      "thu         INTEGER NOT NULL,"\
                      "fri         INTEGER NOT NULL,"\
                      "sat         INTEGER NOT NULL,"\
                      "command     TEXT"\
                  ");"\
                  "CREATE TABLE IF NOT EXISTS rules_turnoff ("\
                      "id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
                      "rule_name   TEXT NOT NULL,"\
                      "time        TEXT NOT NULL,"\
                      "sun         INTEGER NOT NULL,"\
                      "mon         INTEGER NOT NULL,"\
                      "tue         INTEGER NOT NULL,"\
                      "wed         INTEGER NOT NULL,"\
                      "thu         INTEGER NOT NULL,"\
                      "fri         INTEGER NOT NULL,"\
                      "sat         INTEGER NOT NULL,"\
                      "command     TEXT,"\
                      "mode        TEXT NOT NULL"\
                  ");"\
                  "CREATE TABLE IF NOT EXISTS config ("\
                      "id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
                      "options     TEXT,"\
                      "status      INTEGER NOT NULL,"\
                      "version     TEXT,"\
                      "commands    INTEGER NOT NULL,"\
                      "localtime   INTEGER NOT NULL,"\
                      "def_mode    TEXT NOT NULL,"\
                      "boot_time   INTEGER NOT NULL"\
                  ");"\
                  "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat)"\
                  "VALUES ('Example', '10:00:00', 0, 0, 0, 0, 0, 0, 0);"\
                  "INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode)"\
                  "VALUES ('Example', '11:30:00', 0, 0, 0, 0, 0, 0, 0, 'dnf update -y', 'mem');"\
                  "INSERT INTO config (options, status, version, commands, localtime, def_mode, boot_time) VALUES ('-a -v', 1, '" VERSION "', 0, 1, 'off', 300);";

  printf("Opening database...\n");
  // If the database doesn't exist, create and configure it
  if (access(PATH, F_OK) != 0) {
    char *err_msg = 0;

    printf("Database not found, creating it...\n");
    // Check it the directory exists
    printf("[1/5] Creating directory.\n");
    struct stat dir;
    if (stat(DIR, &dir) == -1) {
      // Directory doesn't exist, creating it
      if (mkdir(DIR, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
        return EXIT_FAILURE;

      // Creating logs directory
      mkdir(LOGS, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }

    printf("[2/5] Creating empty file for the database.\n");
    fd = open(PATH, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd == -1) {
      fprintf(stderr, ANSI_COLOR_RED "ERROR: %s\n" ANSI_COLOR_RESET, strerror(errno));
      return EXIT_FAILURE;
    }
    close(fd);

    // Redundancy
    printf("[3/5] Setting directory and file permissions.\n");
    if (chown(DIR, 0, 0) == -1 || chown(PATH, 0, 0) == -1) {
      fprintf(stderr, ANSI_COLOR_RED "ERROR: %s\n" ANSI_COLOR_RESET, strerror(errno));
      return EXIT_FAILURE;
    }

    printf("[4/5] Creating database.\n");
    // Try to open it
    rc = sqlite3_open_v2(PATH, &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc  != SQLITE_OK) {
      remove(PATH);
      fprintf(stderr, ANSI_COLOR_RED "Couldn't create database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(db));
      issue();
      return EXIT_FAILURE;
    }

    printf("[5/5] Configuring database.\n");
    rc = sqlite3_exec(db, SQL, NULL, 0, &err_msg);
    if (rc != SQLITE_OK ) {
      fprintf(stderr, ANSI_COLOR_RED "SQL error: %s\n" ANSI_COLOR_RESET, err_msg);
      issue();
      sqlite3_free(err_msg);
      sqlite3_close(db);
      remove(PATH);
      return EXIT_FAILURE;
    }
    printf(ANSI_COLOR_GREEN "Database created successfully!\n" ANSI_COLOR_RESET);
  }
  return EXIT_SUCCESS;
}
