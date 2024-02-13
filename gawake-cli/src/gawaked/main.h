#ifndef __GAWAKED_H_
#define __GAWAKED_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include "../utils/gawake-types.h"
#include "../utils/validate-rtcwake-args.h"

#include "scheduler.h"
#include "privileges.h"

#define SHUTDOWN "shutdown --poweroff now"

#define COMMAND_BEGINNING "rtcwake "
#define COMMAND_ARGUMENTS "-a -v "
#define COMMAND_TIMESTAMP "--date "
#define COMMAND_MODE " -m "

// YYYYMMDDHHMMSS = 14 characters
// Null terminator = 1
#define COMMAND_LENGTH (strlen (COMMAND_BEGINNING)\
                        + strlen (COMMAND_ARGUMENTS)\
                        + strlen (COMMAND_TIMESTAMP) + 14\
                        + strlen (COMMAND_MODE) + strlen (MODE[rtcwake_args.mode])\
                        + 1)

static void exit_handler (int sig);

#endif /* __GAWAKED_H_ */
