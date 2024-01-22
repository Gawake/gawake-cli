#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "gawake-types.h"

typedef enum
{
  NT_00 = 00,  /* no notification */
  NT_05 = 05,
  NT_15 = 15,
  NT_30 = 30
} NotificationTime;

// Structure for the upcoming turn off rule
typedef struct {
  gboolean found;
  gboolean gawake_status;
  int hour;
  Minutes minutes;
  Mode mode;
  NotificationTime notification_time;
} UpcomingOffRule;

// Structure for the arguments that are passed between parent process an child
// process; these are the rtcwake command arguments
typedef struct {
  gboolean found;
  int hour;
  Minutes minutes;
  int day;
  int month;
  int year;
  Mode mode;
} RtcwakeArgs;

// TODO
//#define pipe_args_s (sizeof (pipe_args_t))
// #define MINUTES_AHEAD 15

#define ALLOC 256
#define BUFFER_ALLOC 5
#define CHECK_DELAY (10 * 60)   /* in seconds */

int scheduler (RtcwakeArgs *args);

#endif /* SCHEDULER_H_ */
