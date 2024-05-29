#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdio.h>

const char *print_timestamp (void);

/*
 * Prints "** DEBUG" magenta colored;
 * then the file, line and function;
 * finally the message passed to the macro
 *
 * USAGE: DEBUG_PRINT (("Var 1: %d, string: %s", var1, str))
 * Outputs to stdout, just for developing debugging reasons
 */
#if PREPROCESSOR_DEBUG
# define DEBUG_PRINT(x) printf ("\x1b[35mDEBUG:\x1b[0m %s:%d:%s(): ", \
                                __FILE__, __LINE__, __func__); \
                                printf x; printf ("\n\n"); fflush (stdout);
#else
# define DEBUG_PRINT(x) /* Don't do anything in release builds */
#endif

/* Similar to previous macro, but also prints the time */
#if PREPROCESSOR_DEBUG
# define DEBUG_PRINT_TIME(x) printf ("\x1b[35mDEBUG:\x1b[0m %s:%d:%s() at %s: ", \
                             __FILE__, __LINE__, __func__, print_timestamp ()); \
                             printf x; printf ("\n\n"); fflush (stdout);
#else
# define DEBUG_PRINT_TIME(x) /* Don't do anything in release builds */
#endif

// Independent of PREPROCESSOR_DEBUG:
# define DEBUG_PRINT_CONTEX  fprintf (stderr, "\x1b[35m** DEBUG:\x1b[0m On %s:%d:%s():\n", \
                             __FILE__, __LINE__, __func__); fflush (stderr)

#endif /* DEBUGGER_H_ */
