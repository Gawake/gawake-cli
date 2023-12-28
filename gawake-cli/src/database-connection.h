#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "gawake.h"
#include "gawake-types.h"

#define DIR	"/var/lib/gawake/"
#define PATH DIR "gawake.db"
#define LOGS DIR "logs/"

int connect_database(sqlite3 **db);
int set_connection (void);
sqlite3 *get_connection (void);
int
 add_rule (Rule rule);

#endif /* DATABASE_CONNECTION_H_ */
