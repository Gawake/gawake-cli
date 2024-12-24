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

#define ALLOW_MANAGING_RULES
#define ALLOW_MANAGING_CONFIGURATION
#include "../database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES
#undef ALLOW_MANAGING_CONFIGURATION

#include "../utils/debugger.h"
#include "../utils/colors.h"

static void menu (void);
static void info (void);
static void clear_buffer (void);
static void get_user_input (Rule *rule, Table table);
static void invalid_value (void);
static void get_int (int *, int, int, int, int);
static int config (void);
static int add_remove_rule (void);
static int confirm (void);
static void usage (void);
static void exit_handler (int);
static int print_rules (Table table);

#endif /* __GAWAKE_CLI_H_ */
