#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "gawake-dbus.h"
#include "gawake-types.h"
#include "database-connection.h"
#include "privileges.h"

static void on_name_acquired (GDBusConnection *connection,
                              const gchar *name,
                              gpointer user_data);

static gboolean
on_handle_add_rule (GawakeServerDatabase          *interface,
                    GDBusMethodInvocation   *invocation,
                    const guint8            hour,
                    const guint8            minutes,
                    const gboolean          day_0,
                    const gboolean          day_1,
                    const gboolean          day_2,
                    const gboolean          day_3,
                    const gboolean          day_4,
                    const gboolean          day_5,
                    const gboolean          day_6,
                    const gchar             *name,
                    const guint8            mode,
                    const guint8            table,
                    gpointer                user_data);

int main (void)
{
  /* TODO */
  /* if (query_gawake_uid ()) */
  /*   return EXIT_FAILURE; */

  /* if (drop_privileges ()) */
  /*   return EXIT_FAILURE; */

  if (set_connection ())
    return EXIT_FAILURE;

  GMainLoop *loop;
  loop = g_main_loop_new (NULL, FALSE);

  g_bus_own_name (G_BUS_TYPE_SESSION,                  // bus type       TODO should it be system wide?
                  "io.github.kelvinnovais.GawakeServer",     // name
                  G_BUS_NAME_OWNER_FLAGS_REPLACE,      // flags
                  NULL,                                // bus_acquired_handler
                  on_name_acquired,                    // name_acquired_handler
                  NULL,                                // name_lost_handler
                  NULL,                                // user_data
                  NULL);                               // user_data_free_func



  g_main_loop_run (loop);

  return EXIT_SUCCESS;
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar *name,
                  gpointer user_data)
{
  // Namespace + interface
  GawakeServerDatabase *interface;
  GError *error;

  interface = gawake_server_database_skeleton_new ();
  g_signal_connect (interface, "handle-add-rule", G_CALLBACK(on_handle_add_rule), NULL);
  //g_signal_connect(interface, "handle-sub", G_CALLBACK(on_handle_sub), NULL);
  error = NULL;
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface),
                                    connection,
                                    "/io/github/kelvinnovais/GawakeServer",
                                    &error);

  /* g_dbus_connection_export_action_group ( */
  /*   connection,                                    */ /* GDBusConnection* connection, */
  /*   "/io/github/kelvinnovais/Gawake",              */ /* const gchar* object_path, */
  /*   NULL,                                          */ /* GActionGroup* action_group, */
  /*   &error                                         */ /* GError** error */
  /* ); */

  //MyDBusAlarm *alarm_interface;
  //alarm_interface = my_dbus_alarm_skeleton_new();
  //g_signal_connect(alarm_interface, "handle-configure-alarm", G_CALLBACK(on_handle_configure_alarm), NULL);
  //error = NULL;
  //g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(alarm_interface), connection, "/", &error);
}

static gboolean
on_handle_add_rule (GawakeServerDatabase          *interface,
                    GDBusMethodInvocation   *invocation,
                    const guint8            hour,
                    const guint8            minutes,
                    const gboolean          day_0,
                    const gboolean          day_1,
                    const gboolean          day_2,
                    const gboolean          day_3,
                    const gboolean          day_4,
                    const gboolean          day_5,
                    const gboolean          day_6,
                    const gchar             *name,
                    const guint8            mode,
                    const guint8            table,
                    gpointer                user_data)
{
  gboolean success;

#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("Hour: %d\n", hour);
  g_print ("Minutes: %d\n", minutes);

  const char DAYS[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
  gboolean days[7] = {day_0, day_1, day_2, day_3, day_4, day_5, day_6};
  g_print ("Days: \n");
  for (gint i = 0; i < 7; i++)
    g_print ("\t%c: %d\n", DAYS[i], days[i]);

  g_print ("Name: %s\n", name);
  g_print ("Mode: %d\n", mode);
  g_print ("Table: %d\n\n\n", table);
#endif

  gRule rule = {
    0,        // not used
    hour,
    minutes,
    {day_0, day_1, day_2, day_3, day_4, day_5, day_6},
    name,
    mode,
    TRUE      // when adding a rule, this value is always true
  };

  if (rule_validated (&rule))
    success = add_rule (&rule);
  else
    success = FALSE;

  gawake_server_database_complete_add_rule (interface, invocation, success);

  return TRUE;
}
