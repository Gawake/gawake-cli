#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "gawake.h"
#include "gawake-types.h"

#define DIR	"/var/lib/gawake/"
#define PATH DIR "gawake.db"
#define LOGS DIR "logs/"

#define ALLOC 256

gint connect_database(sqlite3 **db);
gint set_connection (void);
sqlite3 *get_connection (void);
gboolean rule_validated (gRule *rule); // TODO
gboolean add_rule (const gRule *rule);
gboolean edit_rule (const Rule *rule); // TODO
gboolean delete_rule (int id, int table); // TODO

#endif /* DATABASE_CONNECTION_H_ */
