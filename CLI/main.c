#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>		// Character conversion
#include <unistd.h>		// Check file existence
#include <stdlib.h>		// Run system commands
#include <fcntl.h>		// open()
#include <errno.h>		// Errors handling
#include <pwd.h>		// Get user ID

#include <sys/types.h>	// mkdir()
#include <sys/stat.h>	// mkdir()
#include <sys/wait.h>	// Child process

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Default directory and path to the database:
#define DIR		"/var/gawake/"
#define PATH	DIR "gawake-cli.db"
/*
 * IF YOU ARE GOING TO CHANGE THE DEFAULT VALUES, BE AWARE OF:
 * (1) THE DIR SHOULD NOT BE A SYSTEM DIRECTORY (E.G. INSTEAD OF /var/, USE A SUBFOLDER, LIKE /var/gawake)
 * (2) FOR THE PATH, YOU MUST JUST APPEND THE DATABASE NAME TO THE PREVIOUS 'DIR' VALUE, OTHERWISE YOU'LL GET ERRORS
 */

#define DB_CHECKER "./db_checker"

// Gawake version
#define VERSION	"3.0"

// Remember the effective and real UIDs
static uid_t euid, uid;

struct rules {
	char rule_name[33];
	int time[3];
	int days[7];
	char mode[8];
	char command[129];
};

// DECLARATIONS
void database(sqlite3 **db);
void info(void);
void issue(void);
void exit_on_error(void);
void clear_buffer(void);
void user_input(struct rules *rule, int table);
void invalid_val(void);
void get_int(int *ptr, int digits, int min, int max, int repeat);
void get_gawake_uid(void);
void drop_priv(void);
void raise_priv(void);
int print_table(void *NotUsed, int argc, char **argv, char **azColName);
int print_config(void *NotUsed, int argc, char **argv, char **azColName);
void sqlite_exec_err(int rc, char **err_msg);
int config(sqlite3 **db);
int modify_rule(sqlite3 **db);
int confirm(void);

int main(int argc, char **argv) {
	////////////////// Test zone///////////////////////////

	///////////////////////////////////////////////////////
	/* TODO
	 * Schedule function
	 * Schedule cron helper
	 * Arguments
	 * Turn on command
	 */

	// Getting UIDs
	euid = geteuid();
	get_gawake_uid();

	// Database variables
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

	printf("Starting Gawake...\n");
	database(&db);
	drop_priv();
	printf("···> Choose an option:\n");
	printf("%s", MENU);

	// This do...while loop is a menu: receives the user's choice and stops when the 'q' is entered
	do {
		printf("\n[MENU] ···> ");

		// Receives the user's input
		choice = getchar();

		// Reference [1]
		// A valid answer will have two characters:
		// (1st) a letter in 'choice' and
		// (2nd) a newline on the buffer				like: ('i', '\n')
		// IF the input is not "empty" ('\n') AND the subsequent character entered is not a newline
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
			printf("\n>>>> s\n"); // TODO
			break;
		case 'r':
			printf(ANSI_COLOR_YELLOW "WARNING: Reset database? All rules will be lost!\n" ANSI_COLOR_RESET);
			if (confirm()) {
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
	printf("Report issues: <https://github.com/KelvinNovais/Gawake/issues>\n\n");

	printf("Gawake Copyright (C) 2021 - 2023 Kelvin Novais\nThis program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\".\nThis is free software, and you are welcome to redistribute it under certain conditions; type \"show c\" for details.\n\n");
	printf("(show w/show c/enter to skip) ···> ");
	fgets(choice, 7, stdin);

	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if (strchr(choice, '\n') == NULL)
		clear_buffer();

	if (strcmp(choice, "show w") == 0)
		printf("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
	else if (strcmp(choice, "show c") == 0)
		printf("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. You also can find the license at <https://www.gnu.org/licenses/>\n\n");
}

// Prints the issue URL and related instructions
void issue(void) {
	printf(ANSI_COLOR_RED "If it continues, consider reporting the bug <https://github.com/KelvinNovais/Gawake/issues>\n" ANSI_COLOR_RESET);
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
	const char *TURNOFF_MSG = "[Minutes] (00, 15, 30 or 45) ";
	printf("%-30s", MSG[0]);
	get_int(&( rule -> time[0]), 3, 0, 23, 1);

	// If it's a turn off rule, get the minutes as 0 or 15 or 30 or 45, only; don't get the seconds
	if(table == 2) {
		printf("%-30s", TURNOFF_MSG);
		invalid = 1;
		do {
			get_int(&(rule -> time[1]), 3, 0, 45, 1);
			if (rule -> time[1] == 0
				|| rule -> time[1] == 15
				|| rule -> time[1] == 30
				|| rule -> time[1] == 45)
				invalid = 0;
			else
				invalid_val();

		} while (invalid);
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
	fgets(rule -> command, 129, stdin);
	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if (strchr(rule -> command, '\n') == NULL)
		clear_buffer();
	pch = strstr(rule -> command, "\n");
	if(pch != NULL)
		strncpy(pch, "\0", 1);

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

/**
 * Gets an integer according to:
 * the number of digits wanted;
 * the range (min and max);
 * and if the function must repeat until get a valid value
 *
 * "*ptr" is a pointer to the int variable when the function is called
 **/
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
	struct passwd *p; // TODO: based on the official documentation, is the variable static?
	const char *USER = "gawake";
	if ((p = getpwnam(USER)) == NULL)
		fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: Couldn't get gawake's UID\n" ANSI_COLOR_RESET);
	else
		uid = (int) p -> pw_uid;
}
void drop_priv(void) {
	if (seteuid(uid) != 0)
		fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: Couldn't drop privileges\n" ANSI_COLOR_RESET);
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
	const char *LABELS[] = {"Gawake status: ", "Commands enabled: ", "rtcwake options: "};
	int val = 0;
	for (int i = 0; i < 2; i++) {
		sscanf(argv[i], "%d", &val);
		if (val == 1)
			printf("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_GREEN "Enabled" ANSI_COLOR_RESET);
		else
			printf("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_YELLOW "Disabled" ANSI_COLOR_RESET);
	}
	printf("[%i] %-19s%s\n", 3, LABELS[2], argv[2]);
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

	char *sql = 0, *string = 0;	// to receive a string from the user input
	string = (char *) calloc(1, 98);
	sql = (char *) calloc(1, alloc);
	// Exit if memory allocation fails
	if (sql == NULL || string == NULL) {
		fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: couldn't allocate memory, try again.\n" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	const char *PRINT_CONFIG =	"SELECT status, commands, options FROM config";
	char *err_msg = 0;

	// Get and print the config table
	rc = sqlite3_exec(&(**db), PRINT_CONFIG, print_config, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	// On success, allow user to change its values
	if (rc == SQLITE_OK) {
		printf("To change a value, choose an option (1/2/3/enter to skip) ");
		get_int(&option, 2, 0, 3, 0);
		// If the user wants to change a value, continue
		if (option != 0) {
			// For cases 1 and 2, get the value
			if (option != 3) {
				printf("Enter the new value (0/1) ");
				get_int(&number, 2, 0, 1, 1);
			}
			switch (option) {
			case 1: // STATUS
				snprintf(sql, alloc, "UPDATE config SET status = %d", number);
				break;
			case 2: // COMMANDS
				// Print warning and ask confirmation only if commands are going to be enabled
				if (number == 1)
					printf(ANSI_COLOR_YELLOW "WARNING: All commands are executed as root. Keep in mind:\n"\
						"It's your responsibility which commands you'll run;\nAny verification is done;\nSet the right permissions if you're going to run a script.\n" ANSI_COLOR_RESET);

				if (number == 0 || confirm()) {
					snprintf(sql, alloc, "UPDATE config SET commands = %d", number);
					strcat(sql, string);
				} else {
					return EXIT_SUCCESS;
				}
				break;
			case 3: // RTCWAKE OPTIONS
				// Print warnings, get confirmation; if it's yes, receive the user input (string) and concatenate to the SQL statement
				printf(ANSI_COLOR_YELLOW "ATTENTION: BE CAREFUL while changing the options. Check the rtcwake documentation. DO NOT append the option with commands, otherwise you can DAMAGE your system. Any verification is done.\n" ANSI_COLOR_RESET);
				if (confirm()) {
					printf("rtcwake manpage: <https://www.man7.org/linux/man-pages/man8/rtcwake.8.html>\nGawake default value: \"-a\"\nEnter the options ···> ");
					strcat(sql, "UPDATE config SET options = '");
					fgets(string, 98, stdin);
					// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
					if (strchr(string, '\n') == NULL)
						clear_buffer();
					strcat(sql, string);
					strcat(sql, "'");
				}
			}
			// Insert data, handle errors, free allocated memory
			raise_priv();
			rc = sqlite3_exec(&(**db), sql, NULL, 0, &err_msg);
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
	const char *PRINT_TURNOFF =	"SELECT FORMAT(\"│ %-03i │ %-16.16s│  %s  │  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│ %-30.30s│ %-8s│\", "\
								"id, rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) AS t_off FROM rules_turnoff;";

	// Print turn on/off tables, and handle with possible errors:
	// [1] Turn on table
	printf(ANSI_COLOR_GREEN "[1] TURN ON RULES\n"\
			"┌─────┬─────────────────┬────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────────────────────────────────────────┐"\
			"\n│ %-4s│ %-16s│  Time  │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-40s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command");
	rc = sqlite3_exec(&(**db), PRINT_TURNON, print_table, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	if (rc != SQLITE_OK)
		return EXIT_FAILURE;
	printf("└─────┴─────────────────┴────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────────────────────────────────────────┘\n");

	// [2] Turn off table
	printf(ANSI_COLOR_YELLOW "[2] TURN OFF RULES\n"\
			"┌─────┬─────────────────┬────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬───────────────────────────────┬─────────┐"\
			"\n│ %-4s│ %-16s│  Time  │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-30s│ %-8s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command", "Mode");
	rc = sqlite3_exec(&(**db), PRINT_TURNOFF, print_table, 0, &err_msg);
	sqlite_exec_err(rc, &err_msg);
	if (rc != SQLITE_OK)
		return EXIT_FAILURE;
	printf("└─────┴─────────────────┴────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴───────────────────────────────┴─────────┘\n");

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
			if (sqlite3_prepare_v2(&(**db), sql, -1, &selectstmt, NULL) == SQLITE_OK
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
			if (table == 1)
				snprintf(sql, alloc, "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command) "\
						"VALUES ('%s', '%02d%02d%02d', %d, %d, %d, %d, %d, %d, %d, '%s');",
						rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command);
			else
				snprintf(sql, alloc, "INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) "\
						"VALUES ('%s', '%02d%02d', %d, %d, %d, %d, %d, %d, %d, '%s', '%s');",
						rule.rule_name, rule.time[0], rule.time[1], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, rule.mode);
			break;
		case 2:
			if (table == 1)
				snprintf(sql, alloc, "DELETE FROM rules_turnon WHERE id = %d;", id);
			else
				snprintf(sql, alloc, "DELETE FROM rules_turnoff WHERE id = %d;", id);
			break;
		case 3:
			user_input(&rule, table);
			if (table == 1)
				snprintf(sql, alloc, "UPDATE rules_turnon SET rule_name = '%s', time = '%02d%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, command = '%s' "\
						"WHERE id = %d;",
						rule.rule_name, rule.time[0], rule.time[1], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, id);
			else
				snprintf(sql, alloc, "UPDATE rules_turnoff SET rule_name = '%s', time = '%02d%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, command = '%s', mode = '%s' "\
						"WHERE id = %d;",
						rule.rule_name, rule.time[0], rule.time[1], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6], rule.command, rule.mode, id);
			break;
		}
		raise_priv();
		rc = sqlite3_exec(&(**db), sql, NULL, 0, &err_msg);
		drop_priv();
		sqlite_exec_err(rc, &err_msg);
		free(sql);
	}
	return EXIT_SUCCESS;
}

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

/* REFERENCES:
 * [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
 * [2] https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * [3] https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * 
 * rtcwake manpage: https://www.man7.org/linux/man-pages/man8/rtcwake.8.html
 */

