/* utils.vala
 *
 * Copyright 2023 Kelvin Novais
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

namespace Gawake {
// namespace Utils {

// TODO Since this class is extremely simple, can it be compact (?)
// [Compact (opaque=true)]
public class Utils {
    private static string shared_db_dir;
    private static string shared_db_path;
    private static string user;

    // this code will run exactly once (no need to run more than one time)
    static construct {
        user    = Environment.get_variable ("USER");
        shared_db_dir  = "/home/".concat(user, "/Downloads/gawake-tmp/");
        shared_db_path = shared_db_dir.concat ("test.db");
    }

    public string get_user () {
        return user;
    }

    public string get_shared_db_dir () {
        return shared_db_dir;
    }

    public string get_shared_db_path () {
        return shared_db_path;
    }
}

// } // namespace Utils
} // namespace Gawake

// Compact classes: https://lwildberg.pages.gitlab.gnome.org/vala-tutorial/guides/compact-classes.html
