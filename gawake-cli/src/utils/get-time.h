#ifndef GET_TIME_H_
#define GET_TIME_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int get_time_tm (struct tm **timeinfo);
int get_time (time_t *time_now);

#endif /* GET_TIME_H_ */
