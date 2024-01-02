// SHOULD NOT BE INCLUDED ON OTHER SOURCES FILES BESIDES GAWAKED

#ifndef GAWAKED_H_
#define GAWAKED_H_

static void on_name_acquired (GDBusConnection *connection,
                              const gchar *name,
                              gpointer user_data);

static gboolean
on_handle_add_rule (GawakeServerDatabase    *interface,
                    GDBusMethodInvocation   *invocation,
                    const guint8            hour,
                    const guint8            minutes,
                    const gboolean          day_0,
                    const gboolean          day_1,
                    const gboolean          day_2,
                    const gboolean          day_3,
                    const gboolean          day_4,
                    const gboolean          day_5,
                    const gboolean          day_6,
                    const gchar             *name,
                    const guint8            mode,
                    const guint8            table,
                    gpointer                user_data);

static gboolean
on_handle_edit_rule (GawakeServerDatabase    *interface,
                     GDBusMethodInvocation   *invocation,
                     const guint16           id,
                     const guint8            hour,
                     const guint8            minutes,
                     const gboolean          day_0,
                     const gboolean          day_1,
                     const gboolean          day_2,
                     const gboolean          day_3,
                     const gboolean          day_4,
                     const gboolean          day_5,
                     const gboolean          day_6,
                     const gchar             *name,
                     const guint8            mode,
                     const guint8            table,
                     const gboolean          active,
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

#endif /* GAWAKED_H_ */
