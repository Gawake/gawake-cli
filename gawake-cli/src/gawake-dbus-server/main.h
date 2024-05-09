// SHOULD NOT BE INCLUDED IN OTHER SOURCES FILES BESIDES "gawake-service.c"

#ifndef __GAWAKE_SERVICE_H_
#define __GAWAKE_SERVICE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include "dbus-server.h"

#include "../utils/debugger.h"

static void on_name_acquired (GDBusConnection *connection,
                              const gchar *name,
                              gpointer user_data);

static void on_name_lost (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data);

static gboolean
on_handle_update_database (GawakeServerDatabase    *interface,
                           GDBusMethodInvocation   *invocation,
                           gpointer                user_data);

static gboolean
on_handle_cancel_rule (GawakeServerDatabase    *interface,
                       GDBusMethodInvocation   *invocation,
                       gpointer                user_data);

static gboolean
on_handle_request_schedule (GawakeServerDatabase    *interface,
                            GDBusMethodInvocation   *invocation,
                            gpointer                user_data);

static gboolean
on_handle_request_custom_schedule (GawakeServerDatabase    *interface,
                                   GDBusMethodInvocation   *invocation,
                                   gpointer                user_data);

static gint check_user (void);

static void exit_handler (int sig);

#endif /* __GAWAKE_SERVICE_H_ */
