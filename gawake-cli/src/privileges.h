#ifndef PRIVILEGES_H_
#define PRIVILEGES_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

uid_t get_gawake_uid (void);
int query_gawake_uid (void);
int drop_privileges (void);
int raise_privileges (void);

#endif /* PRIVILEGES_H_ */
