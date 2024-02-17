/* main.c
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

// Main file for gawake-dbus-server

// Defining a debug macro for days
#if PREPROCESSOR_DEBUG
#define DEBUG_DAYS day_0, day_1, day_2, day_3, day_4, day_5, day_6
#endif

#include "main.h"

static GMainLoop *loop;
static guint owner_id;

int main (void)
{
  // Signal for systemd
  signal (SIGTERM, exit_handler);

  loop = g_main_loop_new (NULL, FALSE);

  if (check_user ())
    return EXIT_FAILURE;

  // https://nyirog.medium.com/register-dbus-service-f923dfca9f1
  owner_id = g_bus_own_name (G_BUS_TYPE_SYSTEM,                          // bus type
                             "io.github.kelvinnovais.GawakeServer",      // name
                             G_BUS_NAME_OWNER_FLAGS_REPLACE,             // flags
                             NULL,                                       // bus_acquired_handler
                             on_name_acquired,                           // name_acquired_handler
                             on_name_lost,                               // name_lost_handler
                             NULL,                                       // user_data
                             NULL);                                      // user_data_free_func

  if (connect_database ())
    {
      g_bus_unown_name (owner_id);
      return EXIT_FAILURE;
    }

  g_main_loop_run (loop);

  close_database ();
  g_bus_unown_name (owner_id);

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
  g_signal_connect (interface, "handle-custom-schedule", G_CALLBACK (on_handle_custom_schedule), NULL);
  g_signal_connect (interface, "handle-schedule", G_CALLBACK (on_handle_schedule), NULL);

  error = NULL;
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface),
                                    connection,
                                    "/io/github/kelvinnovais/GawakeServer",
                                    &error);

  if (error != NULL)
    {
      DEBUG_PRINT_CONTEX;
      g_fprintf (stderr, "Couldn't export interface skeleton: %s\n", error->message);
      g_error_free (error);
      close_database ();
      exit (EXIT_FAILURE);
    }
  else
    error = NULL;
}

// If a connection to the bus can’t be made
static void on_name_lost (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data)
{
  DEBUG_PRINT_CONTEX;
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

  DEBUG_PRINT (("RECEIVED:\nName: %s\nHour: %d\nMinutes: %d\n"\
                "Days [SMTWTFS]: %d %d %d %d %d %d %d\nMode: %d\nTable: %d",
                name, hour, minutes, DEBUG_DAYS, mode, table));

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

  if (success)
    gawake_server_database_emit_database_updated (interface);

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

  DEBUG_PRINT (("RECEIVED:\nID: %d\nName: %s\nHour: %d\nMinutes: %d\n"\
                "Days [SMTWTFS]: %d %d %d %d %d %d %d\nActive: %d\nMode: %d\nTable: %d",
                id, name, hour, minutes, DEBUG_DAYS, active, mode, table));

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

  if (success)
    gawake_server_database_emit_database_updated (interface);

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

  DEBUG_PRINT (("RECEIVED:\nID: %d\nTable: %d", id, table));

  success = delete_rule (id, table);

  if (success)
    gawake_server_database_emit_database_updated (interface);

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

  DEBUG_PRINT (("RECEIVED:\nID: %d\nTable: %d\nActive: %d", id, table, active));

  success = enable_disable_rule (id, table, active);

  if (success)
    gawake_server_database_emit_database_updated (interface);

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
  DEBUG_PRINT (("RECEIVED:\nID: %d\nTable: %d", id, table));

  gboolean success;
  // Structure to receive the data; passed as a pointer to the function query_rule
  gRule data = {0, NULL, 0, 0, {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE}, FALSE, MEM, T_ON};

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
      g_print ("Minutes: %u\n", data[i].minutes);

      const char P_DAYS[7] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
      g_print ("Days: \n");
      for (gint j = 0; j < 7; j++)
        {
          g_print ("\t%c: %d\n", P_DAYS[j], (int) data[i].days[j]);
        }

      g_print ("Active: %d\n", data[i].active);
      g_print ("Mode: %u\n", data[i].mode);
      g_print ("Table: %u\n\n", data[i].table);
    }
    g_print ("===========================================\n");
#endif

  // Create GVariant
  // Maybe helpful: https://stackoverflow.com/questions/61996790/gvariantbuilder-build-aii-or-avv
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

static gboolean
on_handle_custom_schedule (GawakeServerDatabase    *interface,
                           GDBusMethodInvocation   *invocation,
                           const guint8            hour,
                           const guint8            minutes,
                           const guint8            day,
                           const guint8            month,
                           const guint16           year,
                           const guint8            mode,
                           gpointer                user_data)
{
  gboolean success;

  DEBUG_PRINT (("RECEIVED (custom schedule):\n"\
                "[HH:MM] %d:%d\n[DD/MM/YYYY] %02d/%02d/%d\nMode: %d",
                hour, minutes, day, month, year, mode));

  success = custom_schedule (hour, minutes, day, month, year, mode);

  if (success)
    gawake_server_database_emit_schedule_requested (interface);

  gawake_server_database_complete_custom_schedule (interface, invocation, success);

  return TRUE;
}

static gboolean
on_handle_schedule (GawakeServerDatabase    *interface,
                    GDBusMethodInvocation   *invocation,
                    gpointer                user_data)
{
  gboolean success = schedule ();

  DEBUG_PRINT (("Received (simple) schedule request"));

  if (success)
    {
      DEBUG_PRINT (("Schedule request signal emitted"));
      gawake_server_database_emit_schedule_requested (interface);
    }


  gawake_server_database_complete_schedule (interface, invocation);

  return TRUE;
}

// Although systemd already takes care of the user that executes the code,
// this function acts as an additional security layer
static gint check_user (void)
{
  uid_t gawake_uid;
  gid_t gawake_gid;

  struct passwd *p;

  // Query gawake user information
  if ((p = getpwnam ("gawake")) == NULL)
    {
      DEBUG_PRINT_CONTEX;
      g_fprintf (stderr, "ERROR: Couldn't query gawake UID\n");
      return EXIT_FAILURE;
    }
  else
    {
      gawake_uid = p->pw_uid;
      gawake_gid = p->pw_gid;
    }

  // Compare results
  if (gawake_uid != getuid () || gawake_gid != getgid ())
    {
      DEBUG_PRINT_CONTEX;
      g_fprintf (stderr, "ERROR: Process not running as gawake user\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static void exit_handler (int sig)
{
  g_main_loop_quit (loop);
  g_bus_unown_name (owner_id);
  close_database ();
  exit (EXIT_SUCCESS);
}
