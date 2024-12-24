/* configuration-reader.h
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

#ifndef CONFIGURATION_READER_H_
#define CONFIGURATION_READER_H_

#include "gawake-types.h"

int configuration_get_localtime (bool *use_localtime);
int configuration_get_default_mode (Mode *default_mode);
int configuration_get_notification_time (int *notification_time);
int configuration_get_shutdown_fail (bool *shutdown_fail);

#endif /* CONFIGURATION_READER_H_ */
