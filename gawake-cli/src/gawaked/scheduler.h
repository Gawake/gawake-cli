#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <time.h>

#include "../utils/gawake-types.h"

// Structure for the upcoming turn off rule
typedef struct {
  bool found;
  bool gawake_status;
  int hour;
  int minutes;
  Mode mode;
  time_t rule_time;
  NotificationTime notification_time;
} UpcomingOffRule;

int scheduler (RtcwakeArgs *rtcwake_args_ptr);

#endif /* SCHEDULER_H_ */
