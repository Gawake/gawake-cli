/* privileges.c
 *
 * Copyright 2021-2024 Kelvin Novais
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#include "privileges.h"
#include "debugger.h"

// Remember the effective and real UIDs
static uid_t gawake_uid, gawake_gid;

// Querying the gawake's uid, an unprivileged user that is used to drop the root privileges
int init_privileges (void)
{
  struct passwd *p;

  if ((p = getpwnam ("gawake")) == NULL)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Couldn't query gawake UID\n");
      return EXIT_FAILURE;
    }
  else
    {
      gawake_uid = p->pw_uid;
      gawake_gid = p->pw_gid;
    }

  return EXIT_SUCCESS;
}

// Temporarily drop root privileges to user "gawake"
int drop_privileges (void)
{
  DEBUG_PRINT (("Dropping privileges"));

  if (setegid (gawake_gid) != 0 || seteuid (gawake_uid) != 0)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Couldn't drop privileges\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int drop_privileges_permanently (void)
{
  DEBUG_PRINT (("Dropping privileges permanently"));

  // Regain privileges...
  if (seteuid (0) != 0)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Couldn't drop privileges\n");
      return EXIT_FAILURE;
    }

  // ...to then drop it
  if (setgid (gawake_gid) != 0 || setuid (gawake_uid) != 0)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Couldn't drop privileges\n");
      return EXIT_FAILURE;
    }

  // Test if it's still possible to regain privileges
  if (setuid (0) != -1)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Managed to regain root privileges?\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

// Raise privileges to root
int raise_privileges (void)
{
  DEBUG_PRINT (("Raiseing privileges"));

  if (seteuid (0) != 0 || setegid (0) != 0)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Couldn't raise privileges\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int check_user (void)
{
  if (gawake_uid != getuid () || gawake_gid != getgid ())
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR: Process not running as gawake user\n");
      return EXIT_FAILURE;
    }

  DEBUG_PRINT (("Check gawake user passed"));

  return EXIT_SUCCESS;
}

/* REFERENCES
 * https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * https://wiki.sei.cmu.edu/confluence/display/c/POS36-C.+Observe+correct+revocation+order+while+relinquishing+privileges
 */
