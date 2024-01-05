/* SYNC */
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "gawake-dbus.h"

int main (void)
{
  GawakeDatabase *proxy;
  GError *error;
  error = NULL;
  gboolean retval;

  proxy = gawake_database_proxy_new_for_bus_sync (
                                                  G_BUS_TYPE_SESSION,
                                                  G_DBUS_PROXY_FLAGS_NONE,
                                                  "io.github.kelvinnovais.Gawake",
                                                  "/io/github/kelvinnovais/Gawake",
                                                  NULL,
                                                  &error
  );

  /* gawake_database_call_add_rule_sync (
   * GawakeDatabase *proxy,
   * int arg_hour,
   * int arg_minutes,
   * int arg_day_0,
   * int arg_day_1,
   * int arg_day_2,
   * int arg_day_3,
   * int arg_day_4,
   * int arg_day_5,
   * int arg_day_6,
   * const int *arg_name,
   * int arg_mode,
   * int arg_table,
   * int *out_success
   *  int *cancellable,
   * int **error) */

    gawake_database_call_add_rule_sync (
                                 proxy,               // proxy
                                 20,                  // arg_hour
                                 15,                  // arg_minutes
                                 TRUE,                // arg_day_0,
                                 FALSE,               // ...
                                 TRUE,
                                 FALSE,
                                 TRUE,
                                 FALSE,
                                 TRUE,                 // arg_day_6
                                 "test client sync",                     // name
                                 0,                                 // mode
                                 0,                                 // table
                                 &retval,                              // success
                                 NULL,           // callback
                                 &error                             // error
                                 );

   if(error == NULL)
        g_print("SUCCESS\n");
    else
        printf("ERROR!\n");

    g_object_unref(proxy);

  return 0;
}
