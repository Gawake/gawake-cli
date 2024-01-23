#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdio.h>

/*
 * Prints "** DEBUG" magenta colored;
 * then the file, line and function;
 * finally the message passed to the macro
 *
 * USAGE: DEBUG_PRINT (("Var 1: %d, string: %s", var1, str))
 */
#if PREPROCESSOR_DEBUG
# define DEBUG_PRINT(x) printf ("\x1b[35m** DEBUG:\x1b[0m %s:%d:%s(): ", \
                                __FILE__, __LINE__, __func__); printf x; printf("\n\n");
#else
# define DEBUG_PRINT(x) /* Don't do anything in release builds */
#endif

/* Similar to previous macro, but also prints the time */
#if PREPROCESSOR_DEBUG
# define DEBUG_PRINT_TIME(x) printf ("\x1b[35m** DEBUG:\x1b[0m %s:%d:%s() [%s %s]: ", \
                                __FILE__, __LINE__, __func__, __DATE__, __TIME__); \
                                printf x; printf("\n\n");
#else
# define DEBUG_PRINT_TIME(x) /* Don't do anything in release builds */
#endif

#endif /* DEBUGGER_H_ */
