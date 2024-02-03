// SHOULD NOT BE INCLUDED IN OTHER SOURCES FILES BESIDES "gawake-service.c"

#ifndef __GAWAKE_SERVICE_H_
#define __GAWAKE_SERVICE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include "../dbus-server.h"
#include "../gawake-types.h"
#include "../database-connection.h"

static void on_name_acquired (GDBusConnection *connection,
                              const gchar *name,
                              gpointer user_data);

static void on_name_lost (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data);

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
                    gpointer                user_data);

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
                     gpointer                user_data);

static gboolean
on_handle_delete_rule (GawakeServerDatabase    *interface,
                       GDBusMethodInvocation   *invocation,
                       const guint16           id,
                       const guint8            table,
                       gpointer                user_data);

static gboolean
on_handle_enable_disable_rule (GawakeServerDatabase    *interface,
                               GDBusMethodInvocation   *invocation,
                               const guint16           id,
                               const guint8            table,
                               const gboolean          active,
                               gpointer                user_data);

static gboolean
on_handle_query_rule (GawakeServerDatabase    *interface,
                      GDBusMethodInvocation   *invocation,
                      const guint16           id,
                      const guint8            table,
                      gpointer                user_data);

static gboolean
on_handle_query_rules (GawakeServerDatabase    *interface,
                       GDBusMethodInvocation   *invocation,
                       const guint8            table,
                       gpointer                user_data);

static gboolean
on_handle_custom_schedule (GawakeServerDatabase    *interface,
                           GDBusMethodInvocation   *invocation,
                           guint8            hour,
                           const guint8            minutes,
                           const guint8            day,
                           const guint8            month,
                           const guint8            year,
                           const guint8            mode,
                           gpointer                user_data);

static gint check_user (void);

static void exit_handler (int sig);

#endif /* __GAWAKE_SERVICE_H_ */
