/* ASYNC */
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "gawake-dbus.h"

static void
callback_add_rule_async(GObject *proxy,
                   GAsyncResult *res,
                   gpointer user_data);

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

  /* gawake_database_call_add_rule (GawakeDatabase *proxy, */
  /*                                int arg_hour, */
  /*                                int arg_minutes, */
  /*                                int arg_day_0, */
  /*                                int arg_day_1, */
  /*                                int arg_day_2, */
  /*                                int arg_day_3, */
  /*                                int arg_day_4, */
  /*                                int arg_day_5, */
  /*                                int arg_day_6, */
  /*                                const int *arg_name, */
  /*                                int arg_mode, */
  /*                                int arg_table, */
  /*                                int *cancellable, */
  /*                                int callback, */
  /*                                int user_data); */

  gawake_database_call_add_rule (
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
                                 "test client",                     // name
                                 0,                                 // mode
                                 0,                                 // table
                                 NULL,                              // cancelable
                                 callback_add_rule_async,           // callback
                                 &error                             // user data (?)
                                 );

   GMainLoop *loop;
   loop = g_main_loop_new(NULL, FALSE);
   g_main_loop_run(loop);

   g_object_unref (proxy);

   return 0;
}

static void
callback_add_rule_async(GObject *proxy,
                   GAsyncResult *res,
                   gpointer user_data)
{
    g_print("callback_add_rule_async called!\n");
    gint retval;
    GError *error;
    error = NULL;
    gawake_database_call_add_rule_finish (GAWAKE_DATABASE (proxy), &retval, res, &error);

    if (error == NULL)
    {
        g_print("Answer = %d\n", retval);
        exit(0);
    }
    else
        g_print("ERROR!!\n");
}
