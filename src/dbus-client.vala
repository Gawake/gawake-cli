/* dbus-client.vala
 *
 * Copyright 2023-2024 Kelvin Novais
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
    public const int RULE_NAME_LENGTH = 33; // Allowed length for rule name

    public enum Week {
        SUNDAY,
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY
    }

    // ATTENTION: must be synced
    public enum Table {
        T_ON,
        T_OFF
    }

    public const string[] TABLE = {
        "rules_turnon",
        "rules_turnoff"
    };

    // ATTENTION: must be synced
    public enum Mode {
        MEM,
        DISK,
        OFF,
        NONE
    }

    public const string[] MODE = {
        "mem",
        "disk",
        "off",
        null
    };

    public enum Minutes {
        M_00 = 00,
        M_15 = 15,
        M_30 = 30,
        M_45 = 45
    }

    public struct Rule {
        uint16 id;
        uint8 hour;
        Minutes minutes;
        bool days[7];
        string name;
        Mode mode;
        bool active;
        uint8 table;
    }

    internal class DBusClient : Object {
        construct {
        }

        public void add_rule_call () {
            // https://valadoc.org/glib-2.0/GLib.Variant.html
            Variant parameters = new Variant(
                "(yybbbbbbbsyy)",
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

            // https://valadoc.org/gio-2.0/GLib.Bus.html
            DBusConnection test = GLib.Bus.get_sync (BusType.SESSION);

            // https://valadoc.org/gio-2.0/GLib.DBusConnection.html
            test.call(
                "io.github.kelvinnovais.GawakeServer", // Bus name
                "/io/github/kelvinnovais/GawakeServer", // Object path
                "io.github.kelvinnovais.Database", // Interface name
                "AddRule", // Method name
                parameters, // Parameters
                null, // Reply type
                GLib.DBusCallFlags.NONE, // Flags
                -1, // Timeout
                null // Cancellable
            );
        } // add_rule_call
    }
} // namespace Gawake
