#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>		// Character conversion
#include <unistd.h>		// Check file existence
#include <stdlib.h>		// Run system commands

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
// TODO remove if not used
//#define ANSI_COLOR_BLUE    "\x1b[34m"
//#define ANSI_COLOR_MAGENTA "\x1b[35m"
//#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Default directory and path to the database:
#define DIR		"/tmp/test/"
#define PATH	"/tmp/test/gawake.db"
/**
 * IF YOU ARE GOING TO CHANGE THE DEFAULT VALUES, BE AWARE OF:
 * (1) THE DIR SHOULD NOT BE A SYSTEM DIRECTORY, INSTEAD OF /var/, USE A SUBFOLDER, LIKE /var/gawake
 * (2) FOR THE PATH, YOU MUST JUST ADD THE DATABASE NAME TO THE PREVIOUS 'DIR' VALUE, OTHERWISE YOU'LL GET ERRORS
 **/

// Gawake version
#define VERSION	"3.0"

// DECLARATIONS
int database(void);
void info(void);
void issue(void);
void clrbff(void);
void user_input(int turnoff);
void invalid_val(void);
void get_int(int *ptr, int digits, int min, int max, int repeat);

int main() {
	char choice = 0;
	int lock = 1;
	printf("Starting Gawake...\n");
	database();
	printf("···> Choose an option:\n");
	printf(	"a\tAdd rule\n"\
			"e\tEdit/remove rule\n"\
			"p\tPrint rules table\n"\
			"s\tSchedule wake up\n\n"\
			"r\tReset database\n"\
			"c\tConfigure Gawake\n"\
			"i\tGawake information\n"\
			"q\tQuit\n\n"
			);

	// This do...while loop is a menu: receives the user's choice and stops when the 'q' is entered
	do{
		printf("[MENU] ···> ");

		// Receives the user's input
		choice = getchar();

		// Reference [1]
		// A valid answer will have two characters:
		// (1st) a letter in 'choice' and
		// (2nd) a newline on the buffer				like: ('i', '\n')
		// IF the input is not "empty" ('\n') AND the subsequent character entered is not a newline
		if( choice != '\n' && getchar() != '\n' ) {
			// flush buffered line and invalidate the input
			clrbff();
			choice = 0 ;
		} else {
			// continue
			choice = tolower(choice);
		}

		switch(choice) {
		case 'a':
			printf("\n>>>> a\n");
			break;
		case 'e':
			printf("\n>>>> e\n");
			break;
		case 'p':
			printf("\n>>>> p\n");
			break;
		case 's':
			printf("\n>>>> s\n");
			break;
		case 'r':
			printf("\n>>>> r\n");
			break;
		case 'c':
			printf("\n>>>> c\n");
			break;
		case 'i':
			info();
			break;
		case 'q':
			printf("Exiting...\n");
			lock = 0;
			break;
		default:
			printf("Choose a valid option!\n");
		}
	} while (lock);

	return 0;
}

// DEFINITIONS
// Connection to the database
int database(void) {
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO
	// local user: turnon_commands: db restricted to that user, no sudo required (/home/{USER}/.gawake-cli/turnon_commands) >>>> init system, call generic script, read user db, process commands
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	sqlite3 *db;		// pointer to the database "object"
   	int rc;				// status of the connection attempt
   	const char *sql =	"CREATE TABLE IF NOT EXISTS rules_turnon ("\
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
							"desktop     INTEGER NOT NULL"\
						");"\
						"INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command)"\
						"VALUES ('Example', '100000', 0, 0, 0, 0, 0, 0, 0, 'echo \"An exemple...\"');"\
						"INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, mode)"\
						"VALUES ('Example', '1030', 0, 0, 0, 0, 0, 0, 0, 'mem');"\
						"INSERT INTO config (options, db_time, status, version, desktop) VALUES ('-a', 'localtime', 1, '" VERSION "', 0);";

   	// If the database doesn't exist, create and configure it; else, just open it
   	if (access(PATH, F_OK) != 0) {
   		char *err_msg = 0;

   		// Database doesn't exist, creating it...
   		printf("Database not found, creating it...\n");

   		printf("[1/5] Creating directory.\n");
   		const char *path_cmd = "sudo mkdir -p " DIR;
   		system(path_cmd);

   		printf("[2/5] Creating empty file for the database.\n");
   		const char *touch_cmd = "sudo touch " PATH;
   		system(touch_cmd);

   		printf("[3/5] Setting directory and file permissions.\n");
   		const char *chmod_cmd = "sudo chmod -R 770 " DIR " " PATH;			// Permissions to read, write and execute for owner and group
   		const char *group_cmd = "sudo groupadd gawake";						// Create a group called gawake
   		const char *chgrp_cmd = "sudo chgrp gawake " DIR " " PATH;			// Change directory and database to gawake group
   		const char *addusr_cmd = "sudo usermod -aG gawake $USER";			// Add the current user to the group
		system(chmod_cmd);
		system(group_cmd);
		system(chgrp_cmd);
		system(addusr_cmd);

		printf("[4/5] Creating database.\n");
		const char *rmdb_cmd = "sudo rm " PATH;								// If fails, will be called above
		// Try to open it
   		rc = sqlite3_open(PATH, &db);
		if(rc  != SQLITE_OK) {
			system(rmdb_cmd);
			fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(db));
			issue();
			exit (1);
		}

		printf("[5/5] Configuring database.\n");
		rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
		if (rc != SQLITE_OK ) {
			fprintf(stderr, ANSI_COLOR_RED "SQL error: %s\n" ANSI_COLOR_RESET, err_msg);
			issue();
			sqlite3_free(err_msg);
			sqlite3_close(db);
			system(rmdb_cmd);
			exit (1);
		}

		printf(ANSI_COLOR_GREEN "Database created successfully!\n" ANSI_COLOR_RESET);

   	} else {
   		// The database is created: just open it
   		rc = sqlite3_open(PATH, &db);
   		if(rc  != SQLITE_OK) {
				fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s.\n" ANSI_COLOR_RESET, sqlite3_errmsg(db));
				issue();
				exit (1);
		} else {
				printf("Opened database successfully\n");
		}
   	}

   	sqlite3_close(db);
   	return 0;
}

// Prints information about Gawake
void info(void) {
	char choice[7];

	printf("\n[INFORMATION]\n");
	printf("Gawake version: %s\n", VERSION);
	printf("SQLite version: %s\n", sqlite3_libversion());
	printf("Report issues: https://github.com/KelvinNovais/Gawake/issues\n\n");
	
	printf("Gawake Copyright (C) 2021 - 2023 Kelvin Novais\nThis program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\".\nThis is free software, and you are welcome to redistribute it under certain conditions; type \"show c\" for details.\n\n");
	printf("(show w/show c/enter to skip) ···> ");
	fgets(choice, 7, stdin);

	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if(strchr(choice, '\n') == NULL) { clrbff(); }

	if(strcmp(choice, "show w") == 0) {
		printf("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
	} else if (strcmp(choice, "show c") == 0){
		printf("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. You also can find the lisense at <https://www.gnu.org/licenses/>\n\n");
	}	
}

// Prints the issue URL and related instructions
void issue(void) {
	printf(ANSI_COLOR_RED "If it continues, consider reporting the bug (https://github.com/KelvinNovais/Gawake/issues)\n" ANSI_COLOR_RESET);
}

// Clears the input buffer
void clrbff(void) {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

void user_input(int turnoff) {
	/**
	 * rule name
	 * time (obs)
	 * days
	 * command
	 * mode (obs)
	 **/
	char rule_name[32];
	int time[3];
	const char *DAYS_NAMES[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	int days[7];
	const char *MODES[] = {"off", "disk", "mem", "standby", "freeze", "no", "on", "disable"};
	int mode = 0;
	char command[128];
	int not_valid = 1; // This variable is a lock for invalid inputs


	// RULE NAME
	// (Do while): repeat if the no name was entered
	printf("\n");
	 do {
		printf("Enter the rule name (can't be null) ···> ");
		fgets(rule_name, 32, stdin);
		// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
		if(strchr(rule_name, '\n') == NULL) { clrbff(); }
	} while (rule_name[0] == '\n') ;

	// TODO TIME
	printf("\nEnter the time rule will be applied:\n");
	const char *MSG[] = {"[Hour] (from 0 to 23) ", "[Minute] (from 0 to 59) ", "[Seconds] (from 0 to 59) "};
	const char *TURNOFF_MSG = "[Minute] (00, 15, 30 or 45) ";
	printf("%*s", -30, MSG[0]);
	get_int(&time[0], 3, 0, 23, 1);

	if(turnoff) {
		// TODO
		printf("%*s", -30, TURNOFF_MSG);
	} else {
		printf("%*s", -30, MSG[1]);
		get_int(&time[1], 3, 0, 59, 1);

		printf("%*s", -30, MSG[2]);
		get_int(&time[2], 3, 0, 59, 1);
	}

	// DAYS
	printf("\nEnter the days the rule will be applied (1 for enabled, and 0 for disabled):\n");
	// For each day of the week, receive the user input
	for (int i = 0; i < 7; i++) {
		printf("%*s", -10, DAYS_NAMES[i]);
		get_int(&days[i], 2, 0, 1, 1);
	}

	// COMMAND
	printf("\nEnter a command (optional, enter to skip) ···> ");
	fgets(command, 128, stdin);
	// Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
	if(strchr(command, '\n') == NULL) { clrbff(); }


	// MODE
	printf("\nSelect a mode:\n");
	for (int i = 0; i < 8; i++) {
		printf("[%i]\t%s\n", i, MODES[i]);
	}
	get_int(&mode, 2, 0, 7, 1);

	exit (0);
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
			if (val >= min && val <= max) {
				printf("min: %d, max: %d\n", min, max);
				invalid = 0;
			}
		}

		// BUT, IF the user's input is longer than expected, invalidate the value
		if(strchr(user_input, '\n') == NULL) {
			if (getchar() != '\n') {
				clrbff();
				invalid = 1;
			}
		}

		if (invalid && repeat) { invalid_val(); } // Print the invalid message only in case the repeat is required (and the input was invalid)
	} while (invalid && repeat); // Only repeat if the function was called with "repeat = 1"
	*ptr = val;
	printf("get_int: %d\n", val);
}

// REFERENCES:
// [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input

