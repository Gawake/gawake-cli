// Main file for gawaked

/* gawaked.c
 *
 * Copyright 2021-2024 Kelvin Novais
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "dbus-server.h"
#include "gawake-types.h"
#include "database-connection.h"
/* #include "privileges.h" */
#include "gawaked.h"

int main (void)
{
  /* TODO */
  /* if (query_gawake_uid ()) */
  /*   return EXIT_FAILURE; */

  /* if (drop_privileges ()) */
  /*   return EXIT_FAILURE; */

  if (connect_database ())
    return EXIT_FAILURE;

  GMainLoop *loop;
  loop = g_main_loop_new (NULL, FALSE);

  g_bus_own_name (G_BUS_TYPE_SESSION,                         // bus type       TODO should it be system wide?
                  "io.github.kelvinnovais.GawakeServer",      // name
                  G_BUS_NAME_OWNER_FLAGS_REPLACE,             // flags
                  NULL,                                       // bus_acquired_handler
                  on_name_acquired,                           // name_acquired_handler
                  on_name_lost,                               // name_lost_handler
                  NULL,                                       // user_data
                  NULL);                                      // user_data_free_func

  g_main_loop_run (loop);

  close_database ();

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

  g_signal_connect (interface, "handle-add-rule", G_CALLBACK (on_handle_add_rule), NULL);
  g_signal_connect (interface, "handle-edit-rule", G_CALLBACK (on_handle_edit_rule), NULL);
  g_signal_connect (interface, "handle-delete-rule", G_CALLBACK (on_handle_delete_rule), NULL);
  g_signal_connect (interface, "handle-enable-disable-rule", G_CALLBACK (on_handle_enable_disable_rule), NULL);
  g_signal_connect (interface, "handle-query-rule", G_CALLBACK (on_handle_query_rule), NULL);
  g_signal_connect (interface, "handle-query-rules", G_CALLBACK (on_handle_query_rules), NULL);

  error = NULL;
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface),
                                    connection,
                                    "/io/github/kelvinnovais/GawakeServer",
                                    &error);

  //MyDBusAlarm *alarm_interface;
  //alarm_interface = my_dbus_alarm_skeleton_new();
  //g_signal_connect(alarm_interface, "handle-configure-alarm", G_CALLBACK(on_handle_configure_alarm), NULL);
  //error = NULL;
  //g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(alarm_interface), connection, "/", &error);
}

// If a connection to the bus can’t be made
static void on_name_lost (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data)
{
  g_fprintf (stderr, "Connection to the bus couldn’t be made\n");
  close_database ();
  exit (EXIT_FAILURE);
}

static gboolean
on_handle_add_rule (GawakeServerDatabase    *interface,
                    GDBusMethodInvocation   *invocation,
                    const gchar             *name,
                    const guint8            hour,
                    const guint8            minutes,
                    const gboolean          day_0,
                    const gboolean          day_1,
                    const gboolean          day_2,
                    const gboolean          day_3,
                    const gboolean          day_4,
                    const gboolean          day_5,
                    const gboolean          day_6,
                    const guint8            mode,
                    const guint8            table,
                    gpointer                user_data)
{
  gboolean success;

#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("Name: %s\n", name);
  g_print ("Hour: %d\n", hour);
  g_print ("Minutes: %d\n", minutes);

  const char P_DAYS[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
  gboolean days[7] = {day_0, day_1, day_2, day_3, day_4, day_5, day_6};
  g_print ("Days: \n");
  for (gint i = 0; i < 7; i++)
    g_print ("\t%c: %d\n", P_DAYS[i], days[i]);

  g_print ("Mode: %d\n", mode);
  g_print ("Table: %d\n\n", table);
#endif

  // Assign "empty" values to the struct
  gRule rule = {
    0,        // id not used, it's autoincremented
    (gchar *) name,
    hour,
    minutes,
    {day_0, day_1, day_2, day_3, day_4, day_5, day_6},
    TRUE,      // when adding a rule, "active" state is always true
    mode,
    table
  };

  // Call the function to add rule, passing the struct pointer
  success = add_rule (&rule);

  gawake_server_database_complete_add_rule (interface, invocation, success);

  return TRUE;
}

static gboolean
on_handle_edit_rule (GawakeServerDatabase    *interface,
                     GDBusMethodInvocation   *invocation,
                     const guint16           id,
                     const gchar             *name,
                     const guint8            hour,
                     const guint8            minutes,
                     const gboolean          day_0,
                     const gboolean          day_1,
                     const gboolean          day_2,
                     const gboolean          day_3,
                     const gboolean          day_4,
                     const gboolean          day_5,
                     const gboolean          day_6,
                     const gboolean          active,
                     const guint8            mode,
                     const guint8            table,
                     gpointer                user_data)
{
  gboolean success;

#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("ID: %d\n", id);
  g_print ("Name: %s\n", name);
  g_print ("Hour: %d\n", hour);
  g_print ("Minutes: %d\n", minutes);

  const char P_DAYS[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
  gboolean days[7] = {day_0, day_1, day_2, day_3, day_4, day_5, day_6};
  g_print ("Days: \n");
  for (gint i = 0; i < 7; i++)
    g_print ("\t%c: %d\n", P_DAYS[i], days[i]);

  g_print ("Active: %d\n\n", active);
  g_print ("Mode: %d\n", mode);
  g_print ("Table: %d\n", table);
#endif

  // Fill struct with received parameters
  gRule rule = {
    id,
    (gchar *) name,
    hour,
    minutes,
    {day_0, day_1, day_2, day_3, day_4, day_5, day_6},
    active,
    mode,
    table
  };

  // Call the edit function, passing the struct pointer
  success = edit_rule (&rule);

  gawake_server_database_complete_edit_rule (interface, invocation, success);

  return TRUE;
}

static gboolean
on_handle_delete_rule (GawakeServerDatabase    *interface,
                      GDBusMethodInvocation   *invocation,
                      const guint16           id,
                      const guint8            table,
                      gpointer                user_data)
{
  gboolean success;

#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("ID: %d\n", id);
  g_print ("Table: %d\n\n", table);
#endif

  success = delete_rule (id, table);

  gawake_server_database_complete_delete_rule (interface, invocation, success);

  return TRUE;
}

// This functions sets the state of the rule: enabled or disabled
static gboolean
on_handle_enable_disable_rule (GawakeServerDatabase    *interface,
                               GDBusMethodInvocation   *invocation,
                               const guint16           id,
                               const guint8            table,
                               const gboolean          active,
                               gpointer                user_data)
{
  gboolean success;

#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("ID: %d\n", id);
  g_print ("Table: %d\n", table);
  g_print ("Active: %d\n\n", active);
#endif

  success = enable_disable_rule (id, table, active);

  gawake_server_database_complete_enable_disable_rule (interface, invocation, success);

  return TRUE;
}

// This function query a single rule
static gboolean
on_handle_query_rule (GawakeServerDatabase    *interface,
                      GDBusMethodInvocation   *invocation,
                      const guint16           id,
                      const guint8            table,
                      gpointer                user_data)
{
#if PREPROCESSOR_DEBUG
  g_print ("RECEIVED:\n");
  g_print ("ID: %d\n", id);
  g_print ("Table: %d\n\n", table);
#endif
  gboolean success;
  // Structure to receive the data; passed as a pointer to the function query_rule
  gRule data = {0, NULL, 0, M_00, {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}, FALSE, MEM, T_ON};

  // This must be allocated to receive the string
  data.name = (gchar *) g_malloc (RULE_NAME_LENGTH);
  //  Assigning a value for the case the ID is invalid, but need to return a response;
  // (avoids Gtk error for NULL string)
  g_snprintf (data.name, ALLOC, "__nothing__");

  // get the data passing the struct pointer
  success = query_rule (id, table, &data);

  // Create a GVariant for the response
  GVariant *rule = g_variant_new ("(qsyybbbbbbbbyy)",
                                  data.id,
                                  data.name,
                                  data.hour,
                                  (guint8) data.minutes,
                                  data.days[0],
                                  data.days[1],
                                  data.days[2],
                                  data.days[3],
                                  data.days[4],
                                  data.days[5],
                                  data.days[6],
                                  data.active,
                                  (guint8) data.mode,
                                  (guint8) data.table
                                  );

  gawake_server_database_complete_query_rule (interface, invocation, rule, success);

  rule = g_variant_ref_sink (rule);
  g_variant_unref (rule);

  return TRUE;
}

// This function queries all rule from the given table
static gboolean
on_handle_query_rules (GawakeServerDatabase    *interface,
                       GDBusMethodInvocation   *invocation,
                       const guint8            table,
                       gpointer                user_data)
{
  gboolean success;
  guint16 rowcount;

  /* Array of structures to receive the data; passed as a pointer to the function query_rules
   * Maybe useful:  https://stackoverflow.com/questions/19948733/dynamically-allocate-memory-for-array-of-structs
   *                https://www.youtube.com/watch?v=lq8tJS3g6tY
   */
  gRule *data = NULL;

  success = query_rules (table, &data, &rowcount);

#if PREPROCESSOR_DEBUG
  g_print ("VALUES:\n");
  g_print ("Row count: %d\n\n", rowcount);
  for (int i = 0; i < rowcount; i++)
    {
      g_print ("===========================================\n");
      g_print ("\nStruct ID: %d\nRule ID: %d\n", i, data[i].id);
      g_print ("Name: %s\n", data[i].name);
      g_print ("Hour: %d\n", data[i].hour);
      g_print ("Minutes: %d\n", data[i].minutes);

      const char P_DAYS[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
      g_print ("Days: \n");
      for (gint j = 0; j < 7; j++)
        {
          g_print ("\t%c: %d\n", P_DAYS[j], (int) data[i].days[j]);
        }

      g_print ("Active: %d\n", data[i].active);
      g_print ("Mode: %d\n", data[i].mode);
      g_print ("Table: %d\n\n", data[i].table);
    }
    g_print ("===========================================\n");
#endif

  // Create GVariant
  // Maybe helful: https://stackoverflow.com/questions/61996790/gvariantbuilder-build-aii-or-avv
  GVariantBuilder builder;
  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(qsyybbbbbbbbyy)"));
  for (gint d = 0; d < rowcount; d++)
    {
      g_variant_builder_add (&builder,
                             "(qsyybbbbbbbbyy)",
                             data[d].id,
                             data[d].name,
                             data[d].hour,
                             data[d].minutes,
                             data[d].days[0],
                             data[d].days[1],
                             data[d].days[2],
                             data[d].days[3],
                             data[d].days[4],
                             data[d].days[5],
                             data[d].days[6],
                             data[d].active,
                             data[d].mode,
                             data[d].table
                             );
    }
  GVariant *rules = g_variant_builder_end (&builder);

  gawake_server_database_complete_query_rules (interface, invocation, rules, success);

  // Free allocated data
  for (gint f = 0; f < rowcount; f++)
    {
      g_free (data[f].name);
    }
  g_free (data);

  rules = g_variant_ref_sink (rules);
  g_variant_unref (rules);

  return TRUE;
}

// TODO should implement SIGKILL or SIGINT?
/* #include <signal.h> */

/* void exit_handler (int sig); */

/* signal (SIGKILL, exit_handler); */

/* void exit_handler (int sig) */
/* { */
/*   g_print ("\nProcess interrupted\n"); */
/*   g_main_loop_quit (); */
/*   g_bus_unown_name() */
/*   close_database (); */
/*   exit (0); */
/* } */
