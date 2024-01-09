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
#include <stdlib.h>

#include <unistd.h>
#include <pwd.h>

#include "privileges.h"

uid_t get_gawake_uid (void);
int query_gawake_uid (void);

// Remember the effective and real UIDs
static uid_t uid;   // euid
static gid_t gid;

uid_t get_gawake_uid (void)
{
  return uid;
}



// Drop root privileges to user "gawake"
int drop_privileges (void)
{


  return EXIT_SUCCESS;
}

// Raise privileges to root
/* int raise_privileges (void) */
/* { */
/*   if (seteuid (0) != 0) */
/*   { */
/*     fprintf (stderr, "ERROR: Couldn't raise privileges\n"); */
/*     return EXIT_FAILURE; */
/*   } */

/*   return EXIT_SUCCESS; */
/* } */

/* REFERENCES
 * https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 */


