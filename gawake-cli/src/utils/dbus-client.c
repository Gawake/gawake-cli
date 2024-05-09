/* dbus-client.c
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

#include <stdio.h>
#include <stdlib.h>

#include "dbus-client.h"
#include "../gawake-dbus-server/dbus-server.h"

#include "colors.h"
#include "debugger.h"

static GawakeServerDatabase *proxy = NULL;
static GError *error = NULL;

gint connect_dbus_client (void)
{
  proxy = gawake_server_database_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,         // bus_type
                                                         G_DBUS_PROXY_FLAGS_NONE,   // flags
                                                         "io.github.kelvinnovais.GawakeServer",  // name
                                                         "/io/github/kelvinnovais/GawakeServer", //object_path
                                                         NULL,                      // cancellable
                                                         &error);                   // error

  if (error != NULL)
    {
      fprintf (stderr, RED ("Unable to get proxy: %s\n"), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

void close_dbus_client (void)
{
  fprintf (stdout, "Closing D-Bus client\n");

  if (error != NULL)
      g_error_free (error);
  else
    error = NULL;

  g_object_unref (proxy);
}

/* 06/01/2024
 * Íres, minha tia, descanse em paz, nós te amamos muito
 */

int trigger_update_database (void)
{
  DEBUG_PRINT (("Update database signal"));

  gawake_server_database_call_update_database_sync (proxy,
                                                    NULL,     // cancellable
                                                    &error);

  if (error != NULL)
    {
      fprintf (stderr,
               YELLOW ("WARNING: Couldn't send database updated signal: %s\n"),
               error->message);

      g_error_free (error);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int trigger_cancel_rule (void)
{
  DEBUG_PRINT (("Cancel rule signal"));

  gawake_server_database_call_cancel_rule_sync (proxy,
                                                NULL,     // cancellable
                                                &error);

  if (error != NULL)
    {
      fprintf (stderr,
               RED ("Error: Couldn't send cancel rule signal: %s\n"),
               error->message);

      g_error_free (error);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int trigger_schedule (void)
{
  DEBUG_PRINT (("Schedule signal"));

  gawake_server_database_call_request_schedule_sync (proxy,
                                                     NULL,     // cancellable
                                                     &error);

  if (error != NULL)
    {
      fprintf (stderr,
               RED ("Error: Couldn't send schedule signal: %s\n"),
               error->message);

      g_error_free (error);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int trigger_custom_schedule (void)
{
  DEBUG_PRINT (("Custom schedule signal"));

  gawake_server_database_call_request_custom_schedule_sync (proxy,
                                                            NULL,     // cancellable
                                                            &error);

  if (error != NULL)
    {
      fprintf (stderr,
               RED ("Error: Couldn't send custom schedule signal: %s\n"),
               error->message);

      g_error_free (error);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
