// Defining types  for gRule struct according to https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types

#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#include <glib.h>

#define RULE_NAME_LENGTH 33    // Allowed length for rule name

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

extern const gchar TABLE[2][13];
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
  M_15 = 15,
  M_30 = 30,
  M_45 = 45
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
