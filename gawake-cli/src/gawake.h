#ifndef GAWAKE_H_
#define GAWAKE_H_

// Defining colors for a better output
#define ANSI_COLOR_RED     "\033[91m"
#define ANSI_COLOR_GREEN   "\033[92m"
#define ANSI_COLOR_YELLOW  "\033[93m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define GREEN(str)    (ANSI_COLOR_GREEN str ANSI_COLOR_RESET)
#define YELLOW(str)   (ANSI_COLOR_YELLOW str ANSI_COLOR_RESET)
#define RED(str)      (ANSI_COLOR_RED str ANSI_COLOR_RESET)

// Apdend with " >> path/turn_off.log 2>&1"
#define LOGS_OUTPUT " >> " LOGS "turn_off.log 2>&1"

#define VERSION	"3.0.2"     // Gawake version

#endif /* GAWAKE_H_ */

