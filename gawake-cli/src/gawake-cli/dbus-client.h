#ifndef DBUS_CLIENT_H_
#define DBUS_CLIENT_H_

#include "../utils/gawake-types.h"
#include "../utils/debugger.h"
#include "../utils/colors.h"

gint connect_dbus_client (void);
void close_dbus_client (void);

// Although these functions have the same name as the database-connection.h, they are not equivalent.
// See that header file for more information
gint add_rule (gRule *rule);
gint delete_rule (guint16 id, Table table);
gint enable_disable_rule (guint16 id, Table table, gboolean active);
gint query_rules (Table table);
gint schedule (void);
gint custom_schedule (int year,
                      int month,
                      int day,
                      int hour,
                      int minutes,
                      int mode);

#endif /* DBUS_CLIENT_H */
