#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "gawake.h"
#include "gawake-types.h"

#define DIR	    "/var/lib/gawake/"
#define PATH        DIR "gawake.db"
#define LOGS        DIR "logs/"           // TODO implement logs?

#define ALLOC 256

// TODO apply GLib Object
// TODO database schema

gint set_connection (void);
sqlite3 *get_connection (void);

gint connect_database(sqlite3 **db);
void close_database (void);

gboolean add_rule (const gRule *rule);
gboolean edit_rule (const gRule *rule);
gboolean delete_rule (guint16 id, Table table);
gboolean enable_disable_rule (guint16 id, Table table, gboolean active);

gboolean query_rule (guint16 id, Table table, gRule *rule);
gboolean query_rules (Table table, gRule **rules, guint16 *rowcount);

/* TODO config database
 * set/get boot delay
 * set/get status
 * set/get dea
 */

#endif /* DATABASE_CONNECTION_H_ */
