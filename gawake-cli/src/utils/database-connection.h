#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include "gawake-types.h"
#include <stdbool.h>

#define ALLOC 256

typedef struct {
  bool use_localtime;
  Mode default_mode;
  int notification_time;
  bool shutdown_fail;
} Config;

// TODO database schema

int connect_database (void);
void close_database (void);

int add_rule (const Rule *rule);
int edit_rule (const Rule *rule);
int delete_rule (const uint16_t id, const Table table);
int enable_disable_rule (const uint16_t id, const Table table, const bool active);

int query_rule (const uint16_t id, const Table table, Rule *rule);
int query_rules (const Table table, Rule **rules, uint16_t *rowcount);

int custom_schedule (const uint8_t  hour,
                     const uint8_t  minutes,
                     const uint8_t  day,
                     const uint8_t  month,
                     const uint16_t year,
                     const uint8_t  mode);

int set_localtime (bool use_localtime);
int set_default_mode (Mode default_mode);
int set_notification_time (int notification_time);
int set_shutdown_fail (bool shutdown_fail);
int get_config (Config *config);

void schedule (void);

#endif /* DATABASE_CONNECTION_H_ */

