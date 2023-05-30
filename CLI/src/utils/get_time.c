#include "get_time.h"

// Get the system time
void get_time(struct tm **timeinfo) {
	time_t rawtime;
	time(&rawtime);
	*timeinfo = localtime(&rawtime);
}
