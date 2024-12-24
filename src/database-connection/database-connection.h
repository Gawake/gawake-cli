/* database-connection.h
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


#ifndef DATABASE_CONNECTION_H_
#define DATABASE_CONNECTION_H_

#include <stdbool.h>
#include <sqlite3.h>

#include "gawake-types.h"

int connect_database (bool read_only);
int disconnect_database (void);

# include "rules-reader.h"

#ifdef ALLOW_MANAGING_RULES
# include "rules-manager.h"
#endif


# include "configuration-reader.h"

#ifdef ALLOW_MANAGING_CONFIGURATION
# include "configuration-manager.h"
#endif

#endif /* DATABASE_CONNECTION_H_ */
