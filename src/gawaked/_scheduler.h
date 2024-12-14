#ifndef SCHEDULER_PRIVATE_H_
#define SCHEDULER_PRIVATE_H_

#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "week-day.h"

#include "../utils/get-time.h"
#include "../utils/debugger.h"
#include "../utils/validate-rtcwake-args.h"

#include "../gawake-dbus-server/dbus-server.h"

// Threads
static void *gsd_listener (void *args);
// static void *login1_listener (void *args);
static void *timed_checker (void *args);
static void finalize_gsd_listener (void);
// static void finalize_login1_listener (void);
static void finalize_timed_checker (void);

// Database calls
static int query_upcoming_off_rule (void);
static int query_upcoming_on_rule (bool use_default_mode);
static int query_custom_schedule (void);

// Signals
static void on_database_updated_signal (void);
static void on_rule_canceled_signal (void);
static void on_schedule_requested_signal (void);
static void on_custom_schedule_requested_signal (void);
// static void
// on_prepare_for_shutdown_signal (GDBusConnection* connection,
//                                 const gchar* sender_name,
//                                 const gchar* object_path,
//                                 const gchar* interface_name,
//                                 const gchar* signal_name,
//                                 GVariant* parameters,
//                                 gpointer user_data);

// Utils
static int day_changed (void);
static void sync_time (void);
static int notify_user (int ret);
static double get_time_remaining (void);
static void schedule_finalize (int ret);

// int take_inhibitor_lock (void);
static void prepare_for_shutdown (int sig);

typedef enum {
  RTCWAKE_ARGS_FAILURE,
  RTCWAKE_ARGS_SUCESS,
  RTCWAKE_ARGS_NOT_FOUND,
  INVALID_RTCWAKE_ARGS
} RtcwakeArgsReturn;

#define ALLOC 256
#define BUFFER_ALLOC 5

// Defining a short sleep time for debug purposes
#if PREPROCESSOR_DEBUG >= 1
#define CHECK_DELAY (10)   /* in seconds */
#else
#define CHECK_DELAY (10 * 60)   /* in seconds */
#endif

#endif /* SCHEDULER_PRIVATE_H_ */
