#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "gawake-types.h"
#include "version.h"

#define DIR	    "/var/lib/gawake/"
#define PATH        DIR "gawake.db"

#define ALLOC 256

// TODO database schema

gint connect_database(void);
void close_database (void);

// Although these functions have the same name as the dbus-client.h, they are not equivalent.
// The following functions are used uniquely on the gawaked service (as a connection to the database);
// in this way, gawaked intermediate the clients (gwake-cli and the graphical application).
// The names are merely mnemonic
gboolean add_rule (const gRule *rule);
gboolean edit_rule (const gRule *rule);
gboolean delete_rule (const guint16 id, const Table table);
gboolean enable_disable_rule (const guint16 id, const Table table, const gboolean active);

gboolean query_rule (const guint16 id, const Table table, gRule *rule);
gboolean query_rules (const Table table, gRule **rules, guint16 *rowcount);

/* TODO config database
 * set/get boot delay
 * set/get status
 */

#endif /* DATABASE_CONNECTION_H_ */

