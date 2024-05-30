#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <time.h>

#include "../utils/gawake-types.h"

// Structure for the upcoming turn off rule
typedef struct {
  bool found;
  /* Only used for processing data */
  bool tomorrow;
  int hour;
  int minutes;
  int day;
  int month;
  int year;
  /*********************************/
  Mode mode;
  time_t rule_time;
  NotificationTime notification_time;
} UpcomingOffRule;

int scheduler (RtcwakeArgs *rtcwake_args_ptr);

#endif /* SCHEDULER_H_ */
