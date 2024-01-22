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

// TODO not used yet
typedef enum
{
  SUNDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
} Week;

// ATTENTION: enum and gchar[] must be synced
typedef enum
{
  T_ON,
  T_OFF
} Table;

extern const char TABLE[2][13];
/////////////////////////////////////////////

// ATTENTION: enum and gchar[] must be synced
typedef enum
{
  MEM,
  DISK,
  OFF
} Mode;

extern const char MODE[3][4];
/////////////////////////////////////////////

typedef enum
{
  M_00 = 00,
  M_10 = 10,
  M_20 = 20,
  M_30 = 30,
  M_40 = 40,
  M_50 = 50
} Minutes;

// Same order as database
typedef struct
{
  guint16 id;                    // q
  // ATTENTION: if not assigned on instantiation, naturally the memory must be allocated
  gchar *name;                   // s
  guint8 hour;                   // y
  Minutes minutes;               // y
  gboolean days[7];              // ab
  gboolean active;               // b
  Mode mode;                     // y
  Table table;                   // y
} gRule;

#endif /* GAWAKE_TYPES_H_ */
