
/*
 * Gawake. A Linux software to make your PC wake up on a scheduled time. It makes the rtcwake command easier.
 * Copyright (C) 2021 - 2023, Kelvin Novais
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, considering ONLY the version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>		// Character conversion
#include <unistd.h>		// Check file existence
#include <stdlib.h>		// Run system commands
#include <errno.h>		// Errors handling
#include <pwd.h>		// Get user ID
#include <time.h>

#include <sys/types.h>	// mkdir()
#include <sys/wait.h>	// Child process

#include "include/gawake.h"
#include "include/get_time.h"
#include "include/wday.h"
#include "include/issue.h"

#define DB_CHECKER "/opt/gawake/bin/cli/db_checker"	// Database helper

// Remember the effective and real UIDs
static uid_t euid, uid;

struct rules {
	char rule_name[33];
	int time[3];
	int days[7];
	char mode[8];
	char command[CMD_LEN];
};

// DECLARATIONS
void database(sqlite3 **db);
void info(void);
void exit_on_error(void);
void clear_buffer(void);
void user_input(struct rules *, int);
void invalid_val(void);
void get_int(int *, int, int, int, int);
void get_gawake_uid(void);
void drop_priv(void);
void raise_priv(void);
int print_table(void *, int, char **, char **);
int print_config(void *, int, char **, char **);
void sqlite_exec_err(int, char **);
int config(sqlite3 **);
int modify_rule(sqlite3 **);
int confirm(void);
void get_time(struct tm **);
int wday(int);
int schedule(sqlite3 **);
void usage(void);

/*
 * TODO Stop on Ctrl C
 * TODO On config: more detailed info
 * TODO (?) Receiving ID when changing rule: loop
*/

int main(int argc, char **argv) {
	// Get UIDs; drop privileges right after
	euid = geteuid();
	get_gawake_uid();
	drop_priv();

	// Database connection object
	sqlite3 *db;

	// Menu variables
	int lock = 1;
	char choice;
	const char *MENU =	"[a]\tAdd/remove/edit rules\n"\
						"[s]\tSchedule wake up\n\n"\
						"[c]\tConfigure Gawake\n"\
						"[i]\tGawake information\n"\
						"[r]\tReset database\n"\
						"[m]\tPrint menu\n"\
						"[q]\tQuit\n";

	// Receiving arguments
	// Reference [4]
	int cflag = 0, mflag = 0, sflag = 0;
	char *cvalue = NULL, *mvalue = NULL;
	int index;
	int c;

	opterr = 0;

	while ((c = getopt(argc, argv, "hsc:m:")) != -1) {
		switch (c) {
		case 'h':
			usage();
			return EXIT_SUCCESS;
		case 's':
			sflag = 1;
			break;
		case 'c':
			cflag = 1;
			cvalue = optarg;
			if (strlen(cvalue) != 14) {
				printf("Invalid time stamp. It must be \"YYYYMMDDhhmmss\".\n");
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			const char *MODES[] = {"off", "disk", "mem", "standby", "freeze", "no", "on", "disable"}; // For checking user input
			int valid = 0;
			mflag = 1;
			mvalue = optarg;
			for (int i = 0; i < 8; i++) {
				if (strcmp(mvalue, MODES[i]) == 0) {
					valid = 1; // If there is a valid mode, continue
					break;
				}
			}
			if (!valid) { // Exit on invalid mode
				printf("Invalid mode\n");
				return EXIT_FAILURE;
			}
			break;
		case '?':
			if (optopt == 'c')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (optopt == 'm')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return EXIT_FAILURE;
		default:
			abort ();
		}
	}

	// Case 's' and 'c', and/or 'm'
	if (sflag && cflag) {
		char cmd[50];
		snprintf(cmd, 50, "sudo rtcwake -a --date %s", cvalue);
		if (mflag) {
			strcat(cmd, " -m ");
			strcat(cmd, mvalue);
		}
		printf("Running rtcwake: %s\n", cmd);
		system(cmd);
		return EXIT_SUCCESS;
	} else if (sflag) { // Case 's', only
		if (mflag) {
			printf("Mode not supported without timestamp (option '-c')\n");
			return EXIT_FAILURE;
		}
		raise_priv();
		if (sqlite3_open_v2(PATH, &db, SQLITE_OPEN_READONLY, NULL)  != SQLITE_OK) {
			fprintf(stderr, "Couldn't open database: %s\n", sqlite3_errmsg(db));
			return EXIT_FAILURE;
		}
		drop_priv();
		if (schedule(&db) == EXIT_FAILURE) {
			sqlite3_close(db);
			exit(EXIT_FAILURE);
		}
	}

	// Exit if there are invalid arguments
	for (index = optind; index < argc; index++)
		printf ("Non-option argument \"%s\"\n", argv[index]);
	if (argc > 1)
		return EXIT_FAILURE;

	// On any arguments
	printf("Starting Gawake...\n");
	raise_priv();
	database(&db);
	drop_priv();
	printf("···> Choose an option:\n");
	printf("%s", MENU);
	// This do...while loop is a menu: receives the user's choice and stops when the 'q' is entered
	do {
		printf("\n[MENU] ···> ");

		// Receives the user's input
		choice = getchar();

		 /* Reference [1]
		 A valid answer will have two characters:
		 (1st) a letter in 'choice' and
		 (2nd) a newline on the buffer				like: ('i', '\n')
		 IF the input is not "empty" ('\n') AND the subsequent character entered is not a newline */
		if (choice != '\n' && getchar() != '\n') {
			// flush buffered line and invalidate the input
			clear_buffer();
			choice = 0 ;
		} else {
			// just continue
			choice = tolower(choice);
		}

		switch (choice) {
		case 'a':
			modify_rule(&db);
			break;
		case 's':
			printf(ANSI_COLOR_YELLOW "ATTENTION: Your computer will turn off now\n" ANSI_COLOR_RESET);
			if (confirm())
				schedule(&db);
			break;
		case 'r':
			printf(ANSI_COLOR_YELLOW "WARNING: Reset database? All rules will be lost!\n" ANSI_COLOR_RESET);
			if (confirm()) {
                        	printf("Removing database...\n");
				sqlite3_close(db);
				raise_priv();
				remove(PATH);
				database(&db);
				drop_priv();
			}
			break;
		case 'c':
			config(&db);
			break;
		case 'i':
			info();
			break;
		case 'm':
			printf("%s", MENU);
			break;
		case 'q':
			printf("Exiting...\n");
			sqlite3_close(db);
			lock = 0;
			break;
		case 'k':
			printf("Your typo resulted in an easter egg: \n\t\"[...] Porque a minha existência não passa de um elétron, perante o Universo.\"\n");
			break;
		default:
			printf("Choose a valid option!\n");
		}
	} while (lock);

	return EXIT_SUCCESS;
}

// DEFINITIONS
// Connection to the database
void database(sqlite3 **db) {
	// Call the database checker
	int rc;
	pid_t pid = fork();
	if (pid == 0) {
		// Child process
		execl(DB_CHECKER, DB_CHECKER, NULL);
		fprintf(stderr, ANSI_COLOR_RED "ERROR (execl): %s\n" ANSI_COLOR_RESET, strerror(errno));
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		// Parent process
		wait(NULL);
	} else {
		fprintf(stderr, ANSI_COLOR_RED "ERROR (fork): %s\n" ANSI_COLOR_RESET, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Open the SQLite database
	rc = sqlite3_open_v2(PATH, db, SQLITE_OPEN_READWRITE, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
		exit(EXIT_FAILURE);
	} else {
		printf(ANSI_COLOR_GREEN "Database opened successfully!\n" ANSI_COLOR_RESET);
	}
}

// Prints information about Gawake
void info(void) {
	char choice[7];

	printf("\n[INFORMATION]\n");
	printf("Gawake version: %s\n", VERSION);
	printf("SQLite version: %s\n", sqlite3_libversion());
	struct tm *timeinfo;
	get_time(&timeinfo);
	printf("Current local time and date: %s", asctime(timeinfo));
	printf("Report issues: <https://github.com/KelvinNovais/Gawake/issues>\n\n");

	printf("Gawake Copyright (C) 2021 - 2023 Kelvin Novais\nThis program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\"."\
			"\nThis is free software, and you are welcome to redistribute it under certain conditions; type \"show c\" for details.\n\n");
	printf("(show w/show c/enter to skip) ···> ");
	fgets(choice, 7, stdin);

	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if (strchr(choice, '\n') == NULL)
		clear_buffer();

	if (strcmp(choice, "show w") == 0)
		printf("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY"\
				"APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT"\
				"HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY"\
				"OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,"\
				"THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR"\
				"PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM"\
				"IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF"\
				"ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
	else if (strcmp(choice, "show c") == 0)
		printf("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. You also can find the license at <https://www.gnu.org/licenses/>\n\n");
}

void exit_on_error(void) {
	fprintf(stderr, ANSI_COLOR_RED "ERROR: %s\n" ANSI_COLOR_RESET, strerror(errno));
	issue();
	if (errno == EACCES)
		printf(ANSI_COLOR_YELLOW "Do you have root privileges?\n" ANSI_COLOR_RESET);
	exit(EXIT_FAILURE);
}

// Clears the input buffer
void clear_buffer(void) {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

// Get the user input, on a valid rule format
void user_input(struct rules *rule, int table) {
	const char *DAYS_NAMES[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	const char *MODES[] = {"off", "disk", "mem", "standby", "freeze", "no", "on", "disable"};
	int mode;
	int invalid = 1;	// This variable is a lock for invalid inputs
	char *pch;			// To get the pointer of a new line char, and after, remove it

	// RULE NAME
	// (Do while): repeat if the no name was entered
	printf("\n");
	do {
		printf("Enter the rule name (can't be null) ···> ");
		fgets(rule -> rule_name, 33, stdin);
		// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
		if (strchr(rule -> rule_name, '\n') == NULL)
			clear_buffer();
	} while (rule -> rule_name[0] == '\n');
	pch = strstr(rule -> rule_name, "\n");
	if(pch != NULL)
		strncpy(pch, "\0", 1);

	// TIME
	printf("\nEnter the time rule will be applied:\n");
	const char *MSG[] = {"[Hour] (from 00 to 23) ", "[Minutes] (from 00 to 59) ", "[Seconds] (from 00 to 59) "};
	const char *TURNOFF_MSG = "[Minutes] (00, 10, 20, 30, 40 or 50) ";
	printf("%-30s", MSG[0]);
	get_int(&( rule -> time[0]), 3, 0, 23, 1);

	// If it's a turn off rule, get the minutes as 0 or 10 or 20 or 30 or 40 or 50, only; seconds ar 00, by default
	if(table == 2) {
		printf("%-30s", TURNOFF_MSG);
		invalid = 1;
		do {
			get_int(&(rule -> time[1]), 3, 0, 50, 1);
			if (rule -> time[1] == 0
				|| rule -> time[1] == 10
				|| rule -> time[1] == 20
				|| rule -> time[1] == 30
				|| rule -> time[1] == 40
				|| rule -> time[1] == 50)
				invalid = 0;
			else
				invalid_val();

		} while (invalid);
                rule -> time[2] = 0;
	} else {
		printf("%-30s", MSG[1]);
		get_int(&(rule -> time[1]), 3, 0, 59, 1);

		printf("%-30s", MSG[2]);
		get_int(&(rule -> time[2]), 3, 0, 59, 1);
	}

	// DAYS
	printf("\nEnter the days the rule will be applied (1 for enabled, and 0 for disabled):\n");
	// For each day of the week, receive the user input
	for (int i = 0; i < 7; i++) {
		printf("%-10s", DAYS_NAMES[i]);
		get_int(&(rule -> days[i]), 2, 0, 1, 1);
	}

	// COMMAND
	printf("\nEnter a command (optional, enter to skip) ···> ");
	fgets(rule -> command, CMD_LEN, stdin);
	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if (strchr(rule -> command, '\n') == NULL)
		clear_buffer();
	pch = strstr(rule -> command, "\n");
	if (pch != NULL)
		strncpy(pch, "\0", 1);
	if (rule -> command[0] == '\n')
		rule -> command[0] = '\0';

	// MODE
	if(table == 2) {
		printf("\nSelect a mode:\n");
		for (int i = 0; i < 8; i++) {
			printf("[%i]\t%s\n", i, MODES[i]);
		}
		get_int(&mode, 2, 0, 7, 1);
		strcpy(rule -> mode, MODES[mode]);
	}
}

// Tell the user that the entered value was invalid
void invalid_val(void) {
	printf(ANSI_COLOR_YELLOW "Please, enter a valid value!\n" ANSI_COLOR_RESET);
}

/*
 * Gets an integer according to:
 * the number of digits wanted;
 * the range (min and max);
 * and if the function must repeat until get a valid value
 *
 * "*ptr" is a pointer to the int variable when the function is called
 */
void get_int(int *ptr, int digits, int min, int max, int repeat) {
	char user_input[digits];	// Number of digits the user input must have
	int val;
	int invalid = 1;			// Boolean
	do {
		printf("···> ");
		// IF the fgets AND the sscanf are successful...
		if ((fgets(user_input, sizeof(user_input), stdin)) && (sscanf(user_input, "%d", &val) == 1)) {
			// ...check if the value is in the range and finishes the loop
			if (val >= min && val <= max)
				invalid = 0;
		}

		// BUT, IF the user's input is longer than expected, invalidate the value
		if(strchr(user_input, '\n') == NULL) {
			if (getchar() != '\n') {
				clear_buffer();
				invalid = 1;
			}
		}

		if (invalid && repeat)
			invalid_val(); // Print the invalid message only in case the repeat is required (and the input was invalid)
	} while (invalid && repeat); // Only repeat if the function was called with "repeat = 1"
	*ptr = val;
}

// Get the gawake's uid, an unprivileged user that is used to drop the root privileges
void get_gawake_uid(void) {
	// Reference [2] and [3]
	struct passwd *p;
	const char *USER = "gawake";
	if ((p = getpwnam(USER)) == NULL) {
        	if (geteuid() == 0) // If coudn't drop privileges, but the user is already unprivileged, don't print the warning
			fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: Couldn't get gawake's UID\n" ANSI_COLOR_RESET);
        } else
		uid = (int) p -> pw_uid;
}
void drop_priv(void) {
	if (seteuid(uid) != 0) {
		if (geteuid() == 0) // If coudn't drop privileges, but the user is already unprivileged, don't print the warning
			fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: Couldn't drop privileges\n" ANSI_COLOR_RESET);
	}
}

void raise_priv(void) {
	if (seteuid(euid) != 0)
		fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: Couldn't raise privileges\n" ANSI_COLOR_RESET);
}

// Print Turn on and turn off tables
int print_table(void *NotUsed, int argc, char **argv, char **azColName) {
	NotUsed = 0;
	printf("%s\n", argv[0]);
	return EXIT_SUCCESS;
}

// Print config table
int print_config(void *NotUsed, int argc, char **argv, char **azColName) {
	NotUsed = 0;
	const char *LABELS[] = {"Gawake status: ", "Commands: ", "Use localtime: ", "rtcwake options: ", "Default mode: "};
	int val = 0;
	for (int i = 0; i < 3; i++) {
		sscanf(argv[i], "%d", &val);
		if (val == 1)
			printf("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_GREEN "Enabled" ANSI_COLOR_RESET);
		else
			printf("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_YELLOW "Disabled" ANSI_COLOR_RESET);
	}
	printf("[%i] %-19s%s\n", 4, LABELS[3], argv[3]);
	printf("[%i] %-19s%s\n", 5, LABELS[4], argv[4]);
	return EXIT_SUCCESS;
}

// Checks if there was an error, prints a message and frees the variable
void sqlite_exec_err(int rc, char **err_msg) {
	if (rc != SQLITE_OK ) {
		fprintf(stderr, ANSI_COLOR_RED "SQL error: %s\n" ANSI_COLOR_RESET, *err_msg);
		issue();
		sqlite3_free(*err_msg);
	}
}

int config(sqlite3 **db) {
	int rc, option, number, alloc = 128;

	char *sql = 0, *string = 0;
	string = (char *) calloc(1, 129);
	sql = (char *) calloc(1, alloc);
	// Exit if memory allocation fails
	if (sql == NULL || string == NULL) {
		fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: couldn't allocate memory, try again.\n" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	const char *PRINT_CONFIG = "SELECT status, commands, localtime, options, def_mode FROM config";
	char *err_msg = 0;

	// Get and print the config table
	rc = sqlite3_exec(*db, PRINT_CONFIG, print_config, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	// On success, allow user to change its values
	if (rc == SQLITE_OK) {
		printf("To change a value, choose an option (1/2/3/4/5/enter to skip) ");
		get_int(&option, 2, 0, 3, 0);
		// If the user wants to change a value, continue
		if (option != 0) {
			if (option < 4) {
				printf("Enter the new value (0/1) ");
				get_int(&number, 2, 0, 1, 1);
			}
			switch (option) {
			case 1: // STATUS
				snprintf(sql, alloc, "UPDATE config SET status = %d;", number);
				break;
			case 2: // COMMANDS
				// Print warning and ask confirmation only if commands are going to be enabled
				if (number == 1)
					printf(ANSI_COLOR_YELLOW "WARNING: All commands are executed as root. Keep in mind:\n"\
						"- It's your responsibility which commands you'll run;\n- Any verification is done;\n- Set the right permissions if you're going to run a script.\n" ANSI_COLOR_RESET);

				if (number == 0 || confirm())
					snprintf(sql, alloc, "UPDATE config SET commands = %d;", number);
				else
					return EXIT_SUCCESS;
				break;
			case 3: // DATABASE TIME
				snprintf(sql, alloc, "UPDATE config SET status = %d;", number);
				break;
			case 4: // RTCWAKE OPTIONS
				// Print warnings, get confirmation; if it's yes, receive the user input (string) and concatenate to the SQL statement
				printf(ANSI_COLOR_YELLOW "ATTENTION: BE CAREFUL while changing the options. Check the rtcwake documentation. "\
						"DO NOT append the options with commands, otherwise you can DAMAGE your system. Any verification is done.\n" ANSI_COLOR_RESET);
				if (confirm()) {
					printf("rtcwake manpage: <https://www.man7.org/linux/man-pages/man8/rtcwake.8.html>"\
							"\nGawake already use \"--date\" and \"-m, --mode\", do not use them.\nGawake default options value: \"-a\"\nEnter the options ···> ");
					strcat(sql, "UPDATE config SET options = '");
					fgets(string, 129, stdin);
					// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
					if (strchr(string, '\n') == NULL)
						clear_buffer();
					char *pch;
					pch = strstr(string, "\n"); // Remove new line character
					if(pch != NULL)
						strncpy(pch, "\0", 1);
					strcat(sql, string);
					strcat(sql, "';");
				}
				break;
			case 5:
				const char *MODES[] = {"off", "disk", "mem", "standby", "freeze", "no", "on", "disable"};
				printf("Select a mode:\n");
				for (int i = 0; i < 8; i++) {
					printf("[%i]\t%s\n", i, MODES[i]);
				}
				get_int(&number, 2, 0, 7, 1);
				snprintf(sql, alloc, "UPDATE config SET def_mode = '%s';", MODES[number]);
				break;
			}
			// Insert data, handle errors, free allocated memory
			raise_priv();
			rc = sqlite3_exec(*db, sql, NULL, 0, &err_msg);
			drop_priv();
			sqlite_exec_err(rc, &err_msg);
		}
	}

	free(sql);
	free(string);
	return EXIT_SUCCESS;
}

int modify_rule(sqlite3 **db) {
	char *err_msg = 0;
	char *sql = 0;
	int rc, option, table, id;
	struct sqlite3_stmt *selectstmt;
	struct rules rule;
	// SQL statements
	const char *PRINT_TURNON =	"SELECT FORMAT(\"│ %-03i │ %-16.16s│ %s │  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│ %-40.40s│\", "\
								"id, rule_name, time, sun, mon, tue, wed, thu, fri, sat, command) AS t_on FROM rules_turnon;";
	const char *PRINT_TURNOFF =	"SELECT FORMAT(\"│ %-03i │ %-16.16s│ %s │  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│ %-30.30s│ %-8s│\", "\
								"id, rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) AS t_off FROM rules_turnoff;";

	// Print turn on/off tables, and handle with possible errors:
	// [1] Turn on table
	printf(ANSI_COLOR_GREEN "[1] TURN ON RULES\n"\
			"┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────────────────────────────────────────┐"\
			"\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-40s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command");
	rc = sqlite3_exec(*db, PRINT_TURNON, print_table, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	if (rc != SQLITE_OK)
		return EXIT_FAILURE;
	printf("└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────────────────────────────────────────┘\n");

	// [2] Turn off table
	printf(ANSI_COLOR_YELLOW "[2] TURN OFF RULES\n"\
			"┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬───────────────────────────────┬─────────┐"\
			"\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-30s│ %-8s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command", "Mode");
	rc = sqlite3_exec(*db, PRINT_TURNOFF, print_table, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	if (rc != SQLITE_OK)
		return EXIT_FAILURE;
	printf("└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴───────────────────────────────┴─────────┘\n");

	printf("Select a table (1/2/enter to skip) ");
	get_int(&table, 2, 0, 2, 0);
	if (table == 1 || table == 2) {
		int alloc = 512;
		sql = (char *) calloc(1, alloc);
		// Exit if memory allocation fails
		if (sql == NULL) {
			fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: couldn't allocate memory, try again.\n" ANSI_COLOR_RESET);
			return EXIT_FAILURE;
		}
		printf("Choose an option:\n[1] Add rule\n[2] Remove rule\n[3] Edit rule\n");
		get_int(&option, 2, 1, 3, 1);
		// If is option 2 or 3, get the ID
		if (option != 1) {
			printf("Enter the ID: ");
			get_int(&id, 5, 1, 9999, 1); // ID between 1 and 9999
			if (table == 1)
				snprintf(sql, alloc, "SELECT id FROM rules_turnon WHERE id = %d;", id);
			else
				snprintf(sql, alloc, "SELECT id FROM rules_turnoff WHERE id = %d;", id);

			// Id the ID doesn't exist, leave
			if (sqlite3_prepare_v2(*db, sql, -1, &selectstmt, NULL) == SQLITE_OK
					&& sqlite3_step(selectstmt) != SQLITE_ROW) {
				printf(ANSI_COLOR_YELLOW "ID not found!\n" ANSI_COLOR_RESET);
				free(sql);
				sqlite3_finalize(selectstmt);
				return EXIT_FAILURE;
			}
		}
		switch (option) {
		case 1:
			user_input(&rule, table);
			if (table == 1) // rules_turnon
				snprintf(sql, alloc, "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command) "\
						"VALUES ('%s', '%02d:%02d:%02d', %d, %d, %d, %d, %d, %d, %d, '%s');",
						rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command);
			else // rules_turnoff
				snprintf(sql, alloc, "INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) "\
						"VALUES ('%s', '%02d:%02d:%02d', %d, %d, %d, %d, %d, %d, %d, '%s', '%s');",
						rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, rule.mode);
			break;
		case 2:
			if (table == 1) // rules_turnon
				snprintf(sql, alloc, "DELETE FROM rules_turnon WHERE id = %d;", id);
			else // rules_turnoff
				snprintf(sql, alloc, "DELETE FROM rules_turnoff WHERE id = %d;", id);
			break;
		case 3:
			user_input(&rule, table);
			if (table == 1) // rules_turnon
				snprintf(sql, alloc, "UPDATE rules_turnon SET rule_name = '%s', time = '%02d:%02d:%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, command = '%s' "\
						"WHERE id = %d;",
						rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, id);
			else // rules_turnoff
				snprintf(sql, alloc, "UPDATE rules_turnoff SET rule_name = '%s', time = '%02d:%02d:%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, command = '%s', mode = '%s' "\
						"WHERE id = %d;",
						rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, rule.mode, id);
			break;
		}
		raise_priv();
		rc = sqlite3_exec(*db, sql, NULL, 0, &err_msg);
		drop_priv();
		sqlite_exec_err(rc, &err_msg);
		free(sql);
	}
	return EXIT_SUCCESS;
}

// Get user's confirmation as 'y'; if confirmed, returns 1
int confirm(void) {
	// Reference [1]
	printf("Do you want to continue? (y/N) ···> ");
	char choice = getchar();
	if (choice != '\n' && getchar() != '\n') {
		// More than one character: flush buffered line and invalidate the input
		clear_buffer();
		choice = 0 ;
	} else {
		choice = tolower(choice);
	}

	if (choice == 'y')
		return 1;
	else
		return 0;
}

/*
 * DESCRIPTION:
 * The time is gotten in  HHMMSS format, and then converted to integer;
 * Time comparison can simply be done as numbers: e.g. 205500 < 233000.
 * Week days follows struct tm wday: 0 to 6, sun to sat
 */
int schedule(sqlite3 **db) {
	int rc, now, db_time = 1, id_match = -1, alloc = 192;

	struct tm *timeinfo;					// Default structure, see documentation
	struct sqlite3_stmt *stmt;

	char time[7];
	const char *DB_TIMES[] = {"utc", "localtime"};
	const char *DAYS[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};	 // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
	char query[alloc], rtcwake_cmd[alloc], buff[7], date[9], mode[8], options[alloc];

	// GET THE DATABASE TIME: 0 FOR UTC AND 1 FOR LOCALTIME
	if (sqlite3_prepare_v2(*db, "SELECT localtime, def_mode, options FROM config WHERE id = 1;", -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed getting config information\n" ANSI_COLOR_RESET);
		issue();
		return EXIT_FAILURE;
	}
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		db_time = sqlite3_column_int(stmt, 0);							// from the database: 0 = utc, 1 = localtime
		snprintf(mode, 8, "%s", sqlite3_column_text(stmt, 1));			// default mode, since this schedule doesn't come from a turn off rule
		snprintf(options, 129, "%s", sqlite3_column_text(stmt, 2));		// rtcwake options
	}
	if (rc != SQLITE_DONE) {
		fprintf(stderr, ANSI_COLOR_RED "ERROR (failed getting config information): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
		issue();
		return EXIT_FAILURE;
	}
	sqlite3_finalize(stmt);

	// GET THE CURRENT TIME
	get_time(&timeinfo);	// hour, minutes and seconds as integer members
	snprintf(buff, 7, "%02d%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec); // Concatenate: HHMMSS as a string
	now = atoi(buff); // HHMMSS as an integer, leading zeros don't care

	// TRY TO SCHEDULE FOR TODAY
	printf("[SCHEDULER] ···> Trying to schedule for today...\n");
	// Create an SQL statement to get today's active rules time; tm_wday = number of the week
	snprintf(query, alloc, "SELECT id, strftime('%%H%%M%%S', time), strftime('%%Y%%m%%d', 'now', '%s') FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC;", DB_TIMES[db_time], DAYS[timeinfo -> tm_wday]);
	rc = sqlite3_prepare_v2(*db, query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for today\n" ANSI_COLOR_RESET);
		issue();
		return EXIT_FAILURE;
	}
	// Get all rules today, ordered by time; the first rule that has a bigger time than now is a valid
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int id			= sqlite3_column_int(stmt, 0);
		int ruletime	= sqlite3_column_int(stmt, 1);
		if (now < ruletime) {
			id_match = id;
			snprintf(date, 9, "%s", sqlite3_column_text(stmt, 2));		// YYYYMMDD
			snprintf(time, 7, "%s", sqlite3_column_text(stmt, 1));		// HHMMSS
			break;
		}
	}
	if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
		fprintf(stderr, ANSI_COLOR_RED "ERROR (failed scheduling for today): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
		issue();
		return EXIT_FAILURE;
	}
	sqlite3_finalize(stmt);

	// IF IT WASN'T POSSIBLE TO SCHEDULE FOR TODAY, TRY ON THE NEXT DAYS
	if (id_match < 0) {
		printf("[SCHEDULER] ···> Any time matched. Trying to schedule for tomorrow or later...\n");
		// search for a matching rule within a week
		for (int i = 1; i <= 7; i++) {
			int wday_num = wday(timeinfo -> tm_wday + i);
			if (wday_num == -1) {
				fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for after (on wday function)\n" ANSI_COLOR_RESET);
				issue();
				return EXIT_FAILURE;
			}
			// The first rule after today is a valid match;
			// also calculate the day: now + number of day until the matching rule, represented by the index i of the loop
			snprintf(query, alloc, "SELECT id, strftime('%%Y%%m%%d', 'now', '%s', '+%d day'), strftime('%%H%%M%%S', time) FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC LIMIT 1;",
					DB_TIMES[db_time], i, DAYS[wday_num]);
			rc = sqlite3_prepare_v2(*db, query, -1, &stmt, NULL);
			if (rc != SQLITE_OK) {
				fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for after\n" ANSI_COLOR_RESET);
				return EXIT_FAILURE;
			}
			while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
				id_match = sqlite3_column_int(stmt, 0);
				snprintf(date, 9, "%s", sqlite3_column_text(stmt, 1));		// YYYYMMDD
				snprintf(time, 7, "%s", sqlite3_column_text(stmt, 2));		// HHMMSS
			}
			if (rc != SQLITE_DONE) {
				fprintf(stderr, ANSI_COLOR_RED "ERROR (failed scheduling for after): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
				issue();
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
		printf(ANSI_COLOR_YELLOW "WARNING: Any turn on rule found. Schedule failed!\n" ANSI_COLOR_RESET);
		return EXIT_SUCCESS;
	}
	// ELSE, SCHEDULE
	sqlite3_close(*db);
	printf("Match on turn on rule with ID [%d]\n", id_match);
	snprintf(rtcwake_cmd, alloc, "rtcwake --date %s%s %s -m %s", date, time, options, mode);
	printf(ANSI_COLOR_GREEN "Running rtcwake: %s\n" ANSI_COLOR_RESET, rtcwake_cmd);
	raise_priv();
	system(rtcwake_cmd);
	drop_priv();			// If the command fails, for any reason
	exit(EXIT_SUCCESS);
}

void usage(void) {
	printf("Gawake: A Linux software to make your PC wake up on a scheduled time. It makes the rtcwake command easier.\n\n"\
			"-c\tSet a custom timestamp (YYYYMMDDhhmmss) for the '-s' option\n"\
			"-h\tShow this help and exit\n"\
			"-m\tSet a mode; must be used together '-c'\n"\
			"-s\tDirectly run the schedule function, based on the first upcoming turn on rule; you can set a different time with the '-d' option\n\n"\
			"Examples:\n"\
			"%-45sSchedule according to the next turn on rule\n"\
			"%-45sSchedule wake for 01 January 2025, at 09:45:00\n"\
			"%-45sSchedule wake for 28 December 2025, at 15:30:00; use mode off\n\n",
			"gawake-cli -s", "gawake-cli -s -d 20250115094500", "gawake-cli -s -d 20251228153000 -m off");
}

/* REFERENCES:
 * [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
 * [2] https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * [3] https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * [4] https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
 *
 * rtcwake manpage: https://www.man7.org/linux/man-pages/man8/rtcwake.8.html
 */

