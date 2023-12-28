// Defining types according to https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types

#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#include <inttypes.h>
#include <stdbool.h>

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

typedef enum
{
  ON,
  OFF
} Mode;

typedef enum
{
  M_00 = 00,
  M_15 = 15,
  M_30 = 30,
  M_45 = 45
} Minutes;

typedef struct
{
  uint16_t id;                    // q
  uint8_t hour;                   // y
  Minutes minutes;                // y
  bool days[7];                   // ab
  // TODO test overbounding this variable
  char name[RULE_NAME_LENGTH];    // s
  Mode mode;                      // y
  bool active;                    // b
} Rule;

#endif /* GAWAKE_TYPES_H_ */
