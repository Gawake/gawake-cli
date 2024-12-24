/* database-connection-utils.h
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

#ifndef DATABASE_CONNECTION_UTILS_H_
#define DATABASE_CONNECTION_UTILS_H_

// #if !defined(DATABASE_CONNECTION_INSIDE)
// # error "Only <database-connection.h> can be included directly."
// #endif

#include "gawake-types.h"
#include <sqlite3.h>

#define ALLOC 256
static char sql[ALLOC];

int validate_rule (const Rule *rule);
int validate_table (const Table table);
int run_sql (void);
sqlite3 * get_pdb (void);
sqlite3 ** get_ppdb (void);

#endif /* DATABASE_CONNECTION_UTILS_H_ */
