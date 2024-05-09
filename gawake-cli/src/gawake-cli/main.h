// SHOULD NOT BE INCLUDED IN OTHER SOURCES FILES BESIDES "gawake-cli.c"

#ifndef __GAWAKE_CLI_H_
#define __GAWAKE_CLI_H_

#include <stdio.h>
#include <ctype.h>          // Character conversion
#include <stdlib.h>         // Run system commands
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// #include <sys/wait.h>       // Child process

#include "../utils/gawake-types.h"
#include "../utils/database-connection.h"
#include "../utils/colors.h"
#include "../utils/debugger.h"

void menu (void);
void info (void);
void clear_buffer (void);
void get_user_input (Rule *rule, Table table);
void invalid_value (void);
void get_int (int *, int, int, int, int);
// int print_config (void *, int, char **, char **);
int config (void);
int add_remove_rule (void);
int confirm (void);
void usage (void);
void exit_handler (int);
void issue (void);
int print_rules (Table table);

#endif /* __GAWAKE_CLI_H_ */
