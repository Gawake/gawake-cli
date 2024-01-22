// SHOULD NOT BE INCLUDED IN OTHER SOURCES FILES BESIDES "gawake-cli.c"

#ifndef __GAWAKE_CLI_H_
#define __GAWAKE_CLI_H_

#include <stdio.h>
#include <ctype.h>          // Character conversion
#include <stdlib.h>         // Run system commands
#include <signal.h>
#include <unistd.h>
#include <glib.h>

#include <sys/wait.h>       // Child process

#include "../gawake-types.h"
#include "../dbus-client.h"
#include "../colors.h"

void menu (void);
void info (void);
void clear_buffer (void);
void get_user_input (gRule *rule, Table table);
void invalid_value (void);
void get_int (gint *, gint, gint, gint, gint);
// gint print_config (void *, int, char **, char **);
gint config (void);
gint add_remove_rule (void);
gint confirm (void);
gint schedule (void);
void usage (void);
void exit_handler (gint);
void issue (void);

#endif /* __GAWAKE_CLI_H_ */

