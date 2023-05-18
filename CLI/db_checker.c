#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Default directory and path to the database:
// FOLLOW SAME INSTRUCTIONS ON main.c
#define DIR		"/var/gawake/"
#define PATH	DIR "gawake-cli.db"

// Gawake version
#define VERSION	"3.0"

void issue(void);

int main(void) {
	int rc, fd;
	sqlite3 *db;
	const char *SQL =	"CREATE TABLE rules_turnon ("\
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
						"CREATE TABLE rules_turnoff ("\
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
						"CREATE TABLE config ("\
							"id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"\
							"options     TEXT,"\
							"db_time     TEXT,"\
							"status      INTEGER NOT NULL,"\
							"version     TEXT,"\
							"commands   INTEGER NOT NULL,"\
							"boot_time   INTEGER NOT NULL"\
						");"\
						"INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command)"\
						"VALUES ('Example', '100000', 0, 0, 0, 0, 0, 0, 0, 'apt update ; apt upgrade -y ; apt autoremove -y');"\
						"INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, mode)"\
						"VALUES ('Example', '1030', 0, 0, 0, 0, 0, 0, 0, 'mem');"\
						"INSERT INTO config (options, db_time, status, version, commands, boot_time) VALUES ('-a', 'localtime', 1, '" VERSION "', 0, 180);";

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
			if (mkdir(DIR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) == -1)
				return EXIT_FAILURE;
		}

		printf("[2/5] Creating empty file for the database.\n");
		fd = open(PATH, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (fd == -1) {
			fprintf(stderr, ANSI_COLOR_RED "ERROR: %s\n" ANSI_COLOR_RESET, strerror(errno));
			return EXIT_FAILURE;
		}
		close(fd);

		printf("[3/5] Setting directory and file permissions.\n");
		if (chown(DIR, 0, 0) == -1
			|| chown(PATH, 0, 0) == -1
			|| chmod(DIR, 0660) == -1
			|| chmod(PATH, 0660) == -1
			) {
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



//
//    fd = open(PATH, O_RDWR);
//    if (fd == -1) {
//    	fprintf(stderr, ANSI_COLOR_RED "ERROR (on db_helper): %s\n" ANSI_COLOR_RESET, strerror(errno));
//        return EXIT_FAILURE;
//    }
//    printf("Database successfully opened (on db_helper)\n");
//    printf("Database file descriptor: %d (on db_helper)\n", fd);
//
//    // Pass the opened file descriptor to the parent process
//    // Here, we use file descriptor number 3 for passing to the parent
//    dup2(fd, 3);
//    close(fd);

    return EXIT_SUCCESS;
}

void issue(void) {
	printf(ANSI_COLOR_RED "If it continues, consider reporting the bug <https://github.com/KelvinNovais/Gawake/issues>\n" ANSI_COLOR_RESET);
}

