#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "gawake-types.h"

typedef struct {
  unsigned short int hour;
  Minutes minutes;
  Mode mode;
} pipe_args_t;

#define pipe_args_s (sizeof (pipe_args_t))

int scheduler (pipe_args_t *args);

#endif /* SCHEDULER_H_ */
