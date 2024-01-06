#ifndef GAWAKE_CLI_H_
#define GAWAKE_CLI_H_

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define GREEN(str)    (ANSI_COLOR_GREEN str ANSI_COLOR_RESET)
#define YELLOW(str)   (ANSI_COLOR_YELLOW str ANSI_COLOR_RESET)
#define RED(str)      (ANSI_COLOR_RED str ANSI_COLOR_RESET)

#include <stdio.h>
#include <string.h>

#include <ctype.h>          // Character conversion
#include <stdlib.h>         // Run system commands
#include <errno.h>          // Errors handling
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <sys/wait.h>       // Child process

#include "gawake-types.h"
#include "version.h"

void menu (void);
void info (void);
void clear_buffer (void);
void get_user_input (gRule *rule, Table table);
void invalid_value (void);
void get_int (gint *, gint, gint, gint, gint);
// gint print_config (void *, int, char **, char **);
gint config (void);
gint modify_rule (void);
gint confirm (void);
gint schedule (void);
void usage (void);
void exit_handler (gint);
void issue (void);

#endif /* GAWAKE_H_ */

