#ifndef GAWAKE_TYPES_H_
#define GAWAKE_TYPES_H_

#define RULE_NAME_LENGTH 33         // Allowed length for rule name
#define MAX_NOTIFICATION_TIME 3600  // In seconds

#define VERSION "3.1.0"

#define DB_NAME "gawake.db"
#define DB_DIR "/var/lib/gawake/"
#define DB_PATH DB_DIR DB_NAME

#include <inttypes.h>
#include <stdbool.h>

// ATTENTION: enum and char[] must be synced
typedef enum
{
  T_ON,
  T_OFF
} Table;

extern const char *TABLE[];
/////////////////////////////////////////////

// ATTENTION: enum and char[] must be synced
typedef enum
{
  MEM,
  DISK,
  OFF,
  NO
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
  uint16_t id;                    // q
  // ATTENTION: if not assigned on instantiation, naturally the memory must be allocated
  char *name;                     // s
  uint8_t hour;                   // y
  uint8_t minutes;                // y
  bool days[7];                   // ab
  bool active;                    // b
  Mode mode;                      // y
  Table table;                    // y
} Rule;

typedef struct {
  bool found;
  bool shutdown_fail;   // This variable is independent of turnon_rules table
  bool run_shutdown;
  int hour;
  int minutes;
  int day;
  int month;
  int year;
  Mode mode;
} RtcwakeArgs;

#define RtcwakeArgs_s sizeof (RtcwakeArgs)

#endif /* GAWAKE_TYPES_H_ */
