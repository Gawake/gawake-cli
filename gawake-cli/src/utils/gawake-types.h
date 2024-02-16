// Defining types  for gRule struct according to https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types

#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#define RULE_NAME_LENGTH 33    // Allowed length for rule name

#define VERSION "3.1.0"

#define DB_NAME "gawake.db"
#define DB_DIR "/var/lib/gawake/"
#define DB_PATH DB_DIR DB_NAME

/*
 * GLib is required wherever there's a D-Bus connection;
 * up to now, all generated binaries has a D-Bus connection.
 *
 * The only element that uses GLib here is gRule; so if at some point this library
 * shouldn't be included (for optimization or whatever), split gRule to another
 * header
 */
#include <glib.h>

// ATTENTION: enum and gchar[] must be synced
typedef enum
{
  T_ON,
  T_OFF
} Table;

extern const char *TABLE[];
/////////////////////////////////////////////

// ATTENTION: enum and gchar[] must be synced
typedef enum
{
  MEM,
  DISK,
  OFF
} Mode;

extern const char *MODE[];
/////////////////////////////////////////////

extern const char *DAYS[];

typedef enum
{
  NT_00 = 00,  /* no notification */
  NT_01 = 01,
  NT_05 = 05,
  NT_15 = 15,
  NT_30 = 30
} NotificationTime;

// Same order as database
typedef struct
{
  guint16 id;                    // q
  // ATTENTION: if not assigned on instantiation, naturally the memory must be allocated
  gchar *name;                   // s
  guint8 hour;                   // y
  guint8 minutes;               // y
  gboolean days[7];              // ab
  gboolean active;               // b
  Mode mode;                     // y
  Table table;                   // y
} gRule;

typedef struct {
  gboolean found;
  gboolean shutdown_fail;   // This variable is independent of turnon_rules table
  gboolean run_shutdown;
  int hour;
  int minutes;
  int day;
  int month;
  int year;
  Mode mode;
} RtcwakeArgs;

#define RtcwakeArgs_s sizeof (RtcwakeArgs)

#endif /* GAWAKE_TYPES_H_ */
