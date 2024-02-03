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

  if ((p = getpwnam ("gawake")) == NULL)
    {
      fprintf (stderr, "ERROR: Couldn't query gawake UID\n");
      return EXIT_FAILURE;
    }
  else
    uid = (int) p -> pw_uid;

  return EXIT_SUCCESS;
}

// Drop root privileges to user "gawake"
int drop_privileges (void)
{
  // TODO add group
  if (seteuid (uid) != 0)
    {
      fprintf (stderr, "ERROR: Couldn't drop privileges\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

// Raise privileges to root
int raise_privileges (void)
{
  // TODO add group
  if (seteuid (0) != 0)
    {
      fprintf (stderr, "ERROR: Couldn't raise privileges\n");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

/* REFERENCES
 * https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 */
