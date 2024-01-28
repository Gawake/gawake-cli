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
#include "dbus-server.h"
#include "colors.h"

static GawakeServerDatabase *proxy;
static GError *error = NULL;

gint connect_dbus_client (void)
{
  proxy = gawake_server_database_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,         // bus_type
                                                         G_DBUS_PROXY_FLAGS_NONE,   // flags
                                                         "io.github.kelvinnovais.GawakeServer",  // name
                                                         "/io/github/kelvinnovais/GawakeServer", //object_path
                                                         NULL,                      // cancellable
                                                         &error);                   // error

  // TODO message if server is not available

  if (error != NULL)
    {
      fprintf (stderr, "Unable to get proxy: %s\n", error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }
  else
    return EXIT_SUCCESS;
}

void close_dbus_client (void)
{
  if (error != NULL)
      g_error_free (error);
  else
    error = NULL;

  g_object_unref (proxy);
}

gint add_rule (gRule *rule)
{
  gboolean success;

  gawake_server_database_call_add_rule_sync (proxy,
                                             rule->name,
                                             rule->hour,
                                             rule->minutes,
                                             rule->days[0],
                                             rule->days[1],
                                             rule->days[2],
                                             rule->days[3],
                                             rule->days[4],
                                             rule->days[5],
                                             rule->days[6],
                                             rule->mode,
                                             rule->table,
                                             &success,    // returned on success
                                             NULL,        // cancellable
                                             NULL);       // error

  if (success)
    {
      printf ("Rule added successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      fprintf (stderr, "Couldn't add rule\n");
      return EXIT_FAILURE;
    }
}

gint delete_rule (guint16 id, Table table)
{
  gboolean success;

  gawake_server_database_call_delete_rule_sync (proxy,
                                                id,
                                                table,
                                                &success,
                                                NULL,       // cancellable
                                                NULL);      // error

  if (success)
    {
      printf ("Rule deleted successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      fprintf (stderr, "Couldn't delete rule\n");
      return EXIT_FAILURE;
    }
}

gint enable_disable_rule (guint16 id, Table table, gboolean active)
{
  gboolean success;

  gawake_server_database_call_enable_disable_rule_sync (proxy,
                                                        id,
                                                        table,
                                                        active,
                                                        &success,
                                                        NULL,     // cancellable
                                                        NULL);    // error

  if (success)
    {
      printf ("Rule state changed successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      fprintf (stderr, "Couldn't change rule state\n");
      return EXIT_FAILURE;
    }
}

/* 06/01/2024
 * Íres, minha tia, descanse em paz, nós te amamos muito
 */

static void print_header (const Table table)
{
  if (table == T_ON)
    {
      printf (GREEN ("[1] TURN ON RULES\n"\
          "┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬────────┐"\
          "\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-7s│"), "ID", "Name", "Active");
    }
  else
    {
      printf (YELLOW ("[2] TURN OFF RULES\n"\
          "┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬────────┬─────────┐"\
          "\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-7s│ %-8s│"), "ID", "Name", "Active", "Mode");
    }
}

static void print_row (const gRule *rule, const Table table)
{
  if (table == T_ON)
    {
      printf ("\n│ %03d │ %-16.15s│  %02d%02u00  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │   %-5d│",
               rule->id, rule->name, rule->hour, rule->minutes,
               rule->days[0], rule->days[1], rule->days[2], rule->days[3], rule->days[4], rule->days[5], rule->days[6],
               rule->active);
    }
  else
    {
      printf ("\n│ %03d │ %-16.15s│  %02d%02u00  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │   %-5d│ %-8s│",
               rule->id, rule->name, rule->hour, rule->minutes,
               rule->days[0], rule->days[1], rule->days[2], rule->days[3], rule->days[4], rule->days[5], rule->days[6],
               rule->active, MODE[rule->mode]);
    }
}

static void print_bottom (const Table table)
{
  if (table == T_ON)
    {
      printf ("\n└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴────────┘\n\n");
    }
  else
    {
      printf ("\n└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴────────┴─────────┘\n");
    }
}

gint query_rules (const Table table)
{
  gboolean success;
  // Since mode and table are guint8 types, they need this kind of variable to
  // receive the data
  guint8 delegate_mode, delegate_table;

  GVariant *rules;
  GVariantIter iter;

  gRule parse;
  parse.name = (gchar *) g_malloc (RULE_NAME_LENGTH);

  gawake_server_database_call_query_rules_sync (proxy,
                                                table,
                                                &rules,
                                                &success,
                                                NULL,     // cancellable
                                                NULL);    // error


  if (success)
    {
      print_header (table);

      // Parse the received data
      g_variant_iter_init (&iter, rules);
      while (g_variant_iter_loop (&iter,
                                  "(qsyybbbbbbbbyy)",
                                  &parse.id,
                                  &parse.name,
                                  &parse.hour,
                                  &parse.minutes,
                                  &parse.days[0],
                                  &parse.days[1],
                                  &parse.days[2],
                                  &parse.days[3],
                                  &parse.days[4],
                                  &parse.days[5],
                                  &parse.days[6],
                                  &parse.active,
                                  &delegate_mode,
                                  &delegate_table))
        {
          parse.mode = (Mode) delegate_mode;
          parse.table = (Table) delegate_table;
          print_row (&parse, table);
        }

      print_bottom (table);
    } // if success
  else
    {
      fprintf (stderr, "Couldn't query rules\n");
    } // else failure

  // Free allocated memory
  g_free (parse.name);
  rules = g_variant_ref_sink (rules);
  g_variant_unref (rules);

  if (success)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
