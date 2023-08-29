#ifndef GAWAKE_H_
#define GAWAKE_H_

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/*
 * Default directory and path to the database:
 * IF YOU ARE GOING TO CHANGE THE DEFAULT VALUES, BE AWARE OF:
 * (1) THE DIR SHOULD NOT BE A SYSTEM DIRECTORY (E.G. INSTEAD OF /var/, USE A SUBFOLDER, LIKE /var/gawake)
 * (2) FOR THE PATH, YOU MUST JUST APPEND THE DATABASE NAME TO THE PREVIOUS 'DIR' VALUE, OTHERWISE YOU'LL GET ERRORS
 */
#define DIR	"/var/gawake/"
#define PATH DIR "gawake-cli.db"
#define LOGS DIR "logs/"
// Apdend with " >> path/turn_off.log 2>&1"
#define LOGS_OUTPUT " >> " LOGS "turn_off.log 2>&1"

#define VERSION	"3.0.2"     // Gawake version
#define CMD_LEN 129         // Allowed length for commands
#define RULE_NAME_LEN 33    // Allowed length for rule name
#define MODE_LEN 8          // Max rtcwake mode length
#define FORMATTED_CMD_LEN (40 + CMD_LEN + MODE_LEN + sizeof(LOGS_OUTPUT) + 100)   // Max estimated length of a whole rtcwake command

#endif /* GAWAKE_H_ */

