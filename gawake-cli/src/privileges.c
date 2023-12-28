#include "privileges.h"
#include "gawake.h"

// Remember the effective and real UIDs
static uid_t euid, uid;

uid_t get_gawake_uid (void)
{
  return uid;
}

// Querying the gawake's uid, an unprivileged user that is used to drop the root privileges
int query_gawake_uid (void)
{
  struct passwd *p;
  const char USER[] = "gawake";

  if ((p = getpwnam(USER)) == NULL)
  {
    fprintf(stderr, RED ("ERROR: Couldn't query gawake UID\n"));
    return EXIT_FAILURE;
  }
  else
    uid = (int) p -> pw_uid;

  return EXIT_SUCCESS;
}

// Drop root privileges to user "gawake"
int drop_privileges (void)
{
  if (seteuid(uid) != 0)
  {
    fprintf(stderr, RED ("ERROR: Couldn't drop privileges\n"));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

// Raise privileges to root
int raise_privileges (void)
{
  if (seteuid(0) != 0)
  {
    fprintf(stderr, RED ("ERROR: Couldn't raise privileges\n"));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* REFERENCES
 * https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 */

