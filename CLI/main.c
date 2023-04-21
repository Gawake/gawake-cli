#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

// Defining colors for a better output
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
//#define ANSI_COLOR_BLUE    "\x1b[34m"
//#define ANSI_COLOR_MAGENTA "\x1b[35m"
//#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Default path to the databse:
#define PATH	"/tmp/test.db"

// DECLARATIONS
int database(void);
void info(void);
void issue(void);
void clrbff(void);

int main() {
	char choice = 0;
	int lock = 1;
	printf("Starting Gawake...\n");
	int db_con = database();
	if(db_con != 0) {
		printf("Exiting...\n");
		return(1);
	}

	printf("···> Choose an option:\n");
	printf("a\tAdd rule\ne\tEdit/remove rule\np\tPrint rules table\ns\tSchedule wake up\n\nr\tReset database\nc\tConfigure Gawake\ni\tGawake information\nq\tQuit\n\n");

	// This do...while loop is a menu: receives the user's choice and stops when the 'q' is entered
	do{
		printf("[MENU] ···> ");

		// Receives the user's input
		choice = getchar();

		// Reference [1] TODO
		// A valid answer will have (i): two characters in 'choice' and (ii) a newline (on the buffer)
		// If the 2nd character on the 'choice' isn't '\n', so the user just pressed enter; it means that the 1st character is '\n'
		if( choice != '\n' && getchar() != '\n' ) {
			clrbff(); // flush buffered line
			choice = 0 ;
		} else {
			choice = tolower(choice);
		}
//		choice = tolower(choice);
//		clrbff();

		switch(choice){
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
	sqlite3 *db;
   	int rc;

	rc = sqlite3_open(PATH, &db);

   	if( rc ) {
      		fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(db));
      		issue();
      		return(1);
   	} else {
      		fprintf(stderr, "Opened database successfully\n");
   	}

   	sqlite3_close(db);
   	return(0);
}

// Prints information about Gawake
void info(void){
	// TODO SQLite version
	// TODO Link to issues page
	
	char choice[7];
	printf("Gawake Copyright (C) 2021 - 2023 Kelvin Novais\nThis program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\".\nThis is free software, and you are welcome to redistribute it under certain conditions; type \"show c\" for details.\n\n");
	printf("···> ");
	fgets(choice, 7, stdin);
	clrbff();

	if(strcmp(choice, "show w") == 0) {
		printf("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
	} else if (strcmp(choice, "show c") == 0){
		printf("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. You also can find the lisense at <https://www.gnu.org/licenses/>\n\n");
	}	
}

// Prints the issue URL and related instructions
void issue(void){
	printf("todo: issue function\n");
}

// Clears the input buffer
void clrbff(void){
	int c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}

// REFERENCES:
// [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
