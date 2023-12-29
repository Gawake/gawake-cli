/*
 * This file is generated by gdbus-codegen, do not modify it.
 *
 * The license of this code is the same as for the D-Bus interface description
 * it was derived from. Note that it links to GLib, so must comply with the
 * LGPL linking clauses.
 */

#ifndef __GAWAKE_DBUS_H__
#define __GAWAKE_DBUS_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for io.github.kelvinnovais.Database */

#define GAWAKE_TYPE_DATABASE (gawake_database_get_type ())
#define GAWAKE_DATABASE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GAWAKE_TYPE_DATABASE, GawakeDatabase))
#define GAWAKE_IS_DATABASE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GAWAKE_TYPE_DATABASE))
#define GAWAKE_DATABASE_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), GAWAKE_TYPE_DATABASE, GawakeDatabaseIface))

struct _GawakeDatabase;
typedef struct _GawakeDatabase GawakeDatabase;
typedef struct _GawakeDatabaseIface GawakeDatabaseIface;

struct _GawakeDatabaseIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_add_rule) (
    GawakeDatabase *object,
    GDBusMethodInvocation *invocation,
    guchar arg_hour,
    guchar arg_minutes,
    gboolean arg_day_0,
    gboolean arg_day_1,
    gboolean arg_day_2,
    gboolean arg_day_3,
    gboolean arg_day_4,
    gboolean arg_day_5,
    gboolean arg_day_6,
    const gchar *arg_name,
    guchar arg_mode,
    guchar arg_table);

};

GType gawake_database_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *gawake_database_interface_info (void);
guint gawake_database_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void gawake_database_complete_add_rule (
    GawakeDatabase *object,
    GDBusMethodInvocation *invocation,
    gboolean success);



/* D-Bus method calls: */
void gawake_database_call_add_rule (
    GawakeDatabase *proxy,
    guchar arg_hour,
    guchar arg_minutes,
    gboolean arg_day_0,
    gboolean arg_day_1,
    gboolean arg_day_2,
    gboolean arg_day_3,
    gboolean arg_day_4,
    gboolean arg_day_5,
    gboolean arg_day_6,
    const gchar *arg_name,
    guchar arg_mode,
    guchar arg_table,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gawake_database_call_add_rule_finish (
    GawakeDatabase *proxy,
    gboolean *out_success,
    GAsyncResult *res,
    GError **error);

gboolean gawake_database_call_add_rule_sync (
    GawakeDatabase *proxy,
    guchar arg_hour,
    guchar arg_minutes,
    gboolean arg_day_0,
    gboolean arg_day_1,
    gboolean arg_day_2,
    gboolean arg_day_3,
    gboolean arg_day_4,
    gboolean arg_day_5,
    gboolean arg_day_6,
    const gchar *arg_name,
    guchar arg_mode,
    guchar arg_table,
    gboolean *out_success,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define GAWAKE_TYPE_DATABASE_PROXY (gawake_database_proxy_get_type ())
#define GAWAKE_DATABASE_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GAWAKE_TYPE_DATABASE_PROXY, GawakeDatabaseProxy))
#define GAWAKE_DATABASE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GAWAKE_TYPE_DATABASE_PROXY, GawakeDatabaseProxyClass))
#define GAWAKE_DATABASE_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GAWAKE_TYPE_DATABASE_PROXY, GawakeDatabaseProxyClass))
#define GAWAKE_IS_DATABASE_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GAWAKE_TYPE_DATABASE_PROXY))
#define GAWAKE_IS_DATABASE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GAWAKE_TYPE_DATABASE_PROXY))

typedef struct _GawakeDatabaseProxy GawakeDatabaseProxy;
typedef struct _GawakeDatabaseProxyClass GawakeDatabaseProxyClass;
typedef struct _GawakeDatabaseProxyPrivate GawakeDatabaseProxyPrivate;

struct _GawakeDatabaseProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  GawakeDatabaseProxyPrivate *priv;
};

struct _GawakeDatabaseProxyClass
{
  GDBusProxyClass parent_class;
};

GType gawake_database_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GawakeDatabaseProxy, g_object_unref)
#endif

void gawake_database_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GawakeDatabase *gawake_database_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
GawakeDatabase *gawake_database_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void gawake_database_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GawakeDatabase *gawake_database_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
GawakeDatabase *gawake_database_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define GAWAKE_TYPE_DATABASE_SKELETON (gawake_database_skeleton_get_type ())
#define GAWAKE_DATABASE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GAWAKE_TYPE_DATABASE_SKELETON, GawakeDatabaseSkeleton))
#define GAWAKE_DATABASE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GAWAKE_TYPE_DATABASE_SKELETON, GawakeDatabaseSkeletonClass))
#define GAWAKE_DATABASE_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GAWAKE_TYPE_DATABASE_SKELETON, GawakeDatabaseSkeletonClass))
#define GAWAKE_IS_DATABASE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GAWAKE_TYPE_DATABASE_SKELETON))
#define GAWAKE_IS_DATABASE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GAWAKE_TYPE_DATABASE_SKELETON))

typedef struct _GawakeDatabaseSkeleton GawakeDatabaseSkeleton;
typedef struct _GawakeDatabaseSkeletonClass GawakeDatabaseSkeletonClass;
typedef struct _GawakeDatabaseSkeletonPrivate GawakeDatabaseSkeletonPrivate;

struct _GawakeDatabaseSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  GawakeDatabaseSkeletonPrivate *priv;
};

struct _GawakeDatabaseSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType gawake_database_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GawakeDatabaseSkeleton, g_object_unref)
#endif

GawakeDatabase *gawake_database_skeleton_new (void);


G_END_DECLS

#endif /* __GAWAKE_DBUS_H__ */
