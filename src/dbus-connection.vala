/* dbus-connection.vala
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

/* MAYBE HELPFUL:
 * https://wiki.gnome.org/Projects/Vala/DBusServerSample
 * https://wiki.gnome.org/Projects/Vala/DBusClientSamples
 */

namespace Gawake {
    const int RULE_NAME_LENGTH = 33; // Allowed length for rule name

    enum Week {
        SUNDAY,
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY
    }

    // ATTENTION: must be synced
    enum Table {
        T_ON,
        T_OFF
    }

    const string[] TABLE = {
        "rules_turnon",
        "rules_turnoff"
    };

    // ATTENTION: must be synced
    enum Mode {
        MEM,
        DISK,
        OFF
    }

    const string[] MODE = {
        "mem",
        "disk",
        "off"
    };

    enum Minutes {
        M_00 = 00,
        M_15 = 15,
        M_30 = 30,
        M_45 = 45
    }

    // struct Rule {
    // uint16 id;
    // uint8 hour;
    // Minutes minutes;
    // bool[] days;
    // string name;
    // Mode mode;
    // bool active;
    // uint8 table;
    // }

    [DBus (name = "io.github.kelvinnovais.Database")]
    public interface Database : Object {
        public abstract bool add_rule (
            uint8 hour,
            uint8 minutes,
            bool day_0,
            bool day_1,
            bool day_2,
            bool day_3,
            bool day_4,
            bool day_5,
            bool day_6,
            string name,
            uint8 mode,
            uint8 table) throws GLib.IOError;
    }

    internal class DBusConnection : Object {
        public Database gdbus = null;

        construct {

            // https://valadoc.org/gio-2.0/GLib.DBusConnection.get_proxy_sync.html
            gdbus = Bus.get_proxy_sync (BusType.SESSION,
                                        "io.github.kelvinnovais.Gawake",
                                        "/io/github/kelvinnovais/Gawake"
                                        );
        }

        public void add_rule_parser () {
            bool ret = gdbus.add_rule (
                10,
                15,
                true,
                false,
                true,
                false,
                true,
                false,
                true,
                "Test Flatpak",
                1,
                1
            );
        }

    }
}
