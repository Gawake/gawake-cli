#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "gawake-dbus.h"
#include "gawake-types.h"
#include "database-connection.h"
#include "privileges.h"

static void on_name_acquired(GDBusConnection *connection,
                             const gchar *name,
                             gpointer user_data);

static gboolean
on_handle_add_rule(GawakeDatabase          *interface,
                   GDBusMethodInvocation   *invocation,
                   const Rule              *rule,
                   gpointer                user_data);

int main(void)
{
  if (query_gawake_uid ())
    return EXIT_FAILURE;

  if (drop_privileges ())
    return EXIT_FAILURE;

  GMainLoop *loop;
  loop = g_main_loop_new(NULL, FALSE);

  g_bus_own_name(G_BUS_TYPE_SESSION,                  // bus type       TODO should it be system wide?
                 "io.github.kelvinnovais.Gawake",     // name
                 G_BUS_NAME_OWNER_FLAGS_REPLACE,      // flags
                 NULL,                                // bus_acquired_handler
                 on_name_acquired,                    // name_acquired_handler
                 NULL,                                // name_lost_handler
                 NULL,                                // user_data
                 NULL);                               // user_data_free_func

  g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
  // Namespace + interface
  GawakeDatabase *interface;
  GError *error;

  interface = gawake_database_skeleton_new ();
  g_signal_connect(interface, "handle-add-rule", G_CALLBACK(on_handle_add_rule), NULL);
  //g_signal_connect(interface, "handle-sub", G_CALLBACK(on_handle_sub), NULL);
  error = NULL;
  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface), connection, "/", &error);

  //MyDBusAlarm *alarm_interface;
  //alarm_interface = my_dbus_alarm_skeleton_new();
  //g_signal_connect(alarm_interface, "handle-configure-alarm", G_CALLBACK(on_handle_configure_alarm), NULL);
  //error = NULL;
  //g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(alarm_interface), connection, "/", &error);
}

static gboolean
on_handle_add_rule(GawakeDatabase          *interface,
                   GDBusMethodInvocation   *invocation,
                   const Rule              *rule,
                   gpointer                user_data)
{
  /* gboolean *success; */
  /* success = true; */
  /* response = g_strdup_printf ("Hello %s.\n", greeting); */
  gawake_database_complete_add_rule (interface, invocation, TRUE);
  g_print("Ok!\n");

  return TRUE;
}
