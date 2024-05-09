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

  g_main_loop_run (loop);

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

  g_signal_connect (interface, "handle-schedule", G_CALLBACK (on_handle_schedule), NULL);

  error = NULL;
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface),
                                    connection,
                                    "/io/github/kelvinnovais/GawakeServer",
                                    &error);

  if (error != NULL)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "Couldn't export interface skeleton: %s\n", error->message);
      g_error_free (error);
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
  fprintf (stderr, "Connection to the bus couldn’t be made\n");
  exit (EXIT_FAILURE);
}

 /* gawake_server_database_emit_database_updated (interface); */

static gboolean
on_handle_database_update (GawakeServerDatabase    *interface,
                           GDBusMethodInvocation   *invocation,
                           gpointer                user_data)
{
  DEBUG_PRINT (("Received update database request"));
  gawake_server_database_complete_update_database (interface, invocation);
  return TRUE;
}

static gboolean
on_handle_cancel_rule (GawakeServerDatabase    *interface,
                       GDBusMethodInvocation   *invocation,
                       gpointer                user_data)
{
  DEBUG_PRINT (("Received cancel rule request"));
  gawake_server_database_complete_cancel_rule (interface, invocation);
  return TRUE;
}

static gboolean
on_handle_schedule (GawakeServerDatabase    *interface,
                    GDBusMethodInvocation   *invocation,
                    gpointer                user_data)
{
  DEBUG_PRINT (("Received schedule request"));

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
      fprintf (stderr, "ERROR: Couldn't query gawake UID\n");
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
      fprintf (stderr, "ERROR: Process not running as gawake user\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static void exit_handler (int sig)
{
  g_main_loop_quit (loop);
  g_bus_unown_name (owner_id);
  exit (EXIT_SUCCESS);
}
