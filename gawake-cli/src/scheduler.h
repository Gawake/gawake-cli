#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <time.h>

#include "gawake-types.h"

// Structure for the upcoming turn off rule
typedef struct {
  gboolean found;
  gboolean gawake_status;
  int hour;
  Minutes minutes;
  Mode mode;
  time_t rule_time;
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

// Defining a short sleep time for debug purposes
#if PREPROCESSOR_DEBUG
#define CHECK_DELAY (10)   /* in seconds */
#else
#define CHECK_DELAY (10 * 60)   /* in seconds */
#endif

int scheduler (RtcwakeArgs *rtcwake_args_ptr);

#endif /* SCHEDULER_H_ */
