// Defining types  for gRule struct according to https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types

#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#define RULE_NAME_LENGTH 33    // Allowed length for rule name

#define VERSION "3.1.0"

#define DB_NAME "gawake.db"
#define DB_DIR "/var/lib/gawake/"
#define DB_PATH DB_DIR DB_NAME

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

// TODO add minutes in range of 10
typedef enum
{
  M_00 = 00,
  M_15 = 15,
  M_30 = 30,
  M_45 = 45
} Minutes;

#endif /* GAWAKE_TYPES_H_ */
