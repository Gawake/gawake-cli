// Defining types according to https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types

#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#include <stdbool.h>
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

// ATTENTION: must be synced
typedef enum
{
  T_ON,
  T_OFF
} Table;

static const char *TABLE[] = {
  "rules_turnon",
  "rules_turnoff"
};

// ATTENTION: must be synced
typedef enum
{
  MEM,
  DISK,
  OFF
} Mode;

static const char *MODE[] = {
  "mem",
  "disk",
  "off"
};

typedef enum
{
  M_00 = 00,
  M_15 = 15,
  M_30 = 30,
  M_45 = 45
} Minutes;

typedef struct
{
  guint16 id;                    // q
  guint8 hour;                   // y
  Minutes minutes;               // y
  gboolean days[7];              // ab
  gchar *name;                   // s
  Mode mode;                     // y
  gboolean active;               // b
  guint8 table;                  // y
} gRule;

#endif /* GAWAKE_TYPES_H_ */
