#ifndef GAWAKE_GTYPES_H_
#define GAWAKE_GTYPES_H_

#include <glib.h>

// Same order as database
typedef struct
{
  guint16 id;                    // q
  // ATTENTION: if not assigned on instantiation, naturally the memory must be allocated
  gchar *name;                   // s
  guint8 hour;                   // y
  Minutes minutes;               // y
  gboolean days[7];              // ab
  gboolean active;               // b
  Mode mode;                     // y
  Table table;                   // y
} gRule;

#endif /* GAWAKE_GTYPES_H_ */
