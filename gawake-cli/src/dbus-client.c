/* SYNC */

#include "dbus-server.h"
#include "dbus-client.h"

static GawakeServerDatabase *proxy;
static GError *error = NULL;

gint connect_dbus_client (void)
{
  gboolean retval;

  proxy = gawake_server_database_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,        // bus_type
                                                         G_DBUS_PROXY_FLAGS_NONE,   // flags
                                                         "io.github.kelvinnovais.GawakeServer",  // name
                                                         "/io/github/kelvinnovais/GawakeServer", //object_path
                                                         NULL,                      // cancellable
                                                         &error);                   // error

  if (error != NULL)
    {
      g_fprintf (stderr, "Unable to get proxy: %s\n", error -> message);
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
                                             rule -> name,
                                             rule -> hour,
                                             rule -> minutes,
                                             rule -> days[0],
                                             rule -> days[1],
                                             rule -> days[2],
                                             rule -> days[3],
                                             rule -> days[4],
                                             rule -> days[5],
                                             rule -> days[6],
                                             rule -> mode,
                                             rule -> table,
                                             &success,    // returned on success
                                             NULL,        // cancellable
                                             NULL);       // error

  if (success)
    {
      gprinf ("Rule added successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      gprinf ("Couldn't add rule\n");
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
      gprinf ("Rule deleted successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      gprinf ("Couldn't delete rule\n");
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
      gprinf ("Rule state changed successfully!\n");
      return EXIT_SUCCESS;
    }
  else
    {
      gprinf ("Couldn't change rule state\n");
      return EXIT_FAILURE;
    }
}

gint query_rules (Table table)
{
  gboolean success;

  // TODO

  if (success)
      return EXIT_SUCCESS;
  else
    {
      gprinf ("Couldn't query rules\n");
      return EXIT_FAILURE;
    }
}
