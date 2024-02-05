#ifndef SCHEDULER_PRIVATE_H_
#define SCHEDULER_PRIVATE_H_

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "../get-time.h"
#include "../week-day.h"
#include "../debugger.h"
#include "../dbus-server.h"
#include "../validate-rtcwake-args.h"

// Threads
static void *dbus_listener (void *args);
static void *timed_checker (void *args);
static void finalize_dbus_listener (void);
static void finalize_timed_checker (void);

// Database calls
static int query_upcoming_off_rule (void);  // TODO treat on fail (?)
static int query_upcoming_on_rule (gboolean use_default_mode);
static int query_custom_schedule (void);

// Signals
static void on_database_updated_signal (void);
static void on_rule_canceled_signal (void);
static void on_schedule_requested_signal (void);
static void exit_handler (int sig);

// Utils
static int day_changed (void);
static void sync_time (void);
static void notify_user (int ret);
static double get_time_remaining (void);

typedef enum {
  RTCWAKE_ARGS_FAILURE,
  RTCWAKE_ARGS_SUCESS,
  RTCWAKE_ARGS_NOT_FOUND,
  INVALID_RTCWAKE_ARGS
} RtcwakeArgsReturn;

#define ALLOC 256
#define BUFFER_ALLOC 5

// Defining a short sleep time for debug purposes
#if PREPROCESSOR_DEBUG
#define CHECK_DELAY (10)   /* in seconds */
#else
#define CHECK_DELAY (10 * 60)   /* in seconds */
#endif

#endif /* SCHEDULER_PRIVATE_H_ */
