/* rule-setup-dialog.vala
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
    [GtkTemplate (ui = "/com/kelvinnovais/Gawake/ui/rule-setup-dialog.ui")]
    public class RuleSetupDialog : Adw.Window {
        [GtkChild]
        private unowned Gtk.SpinButton h_spinbutton;
        [GtkChild]
        private unowned Gtk.SpinButton m_spinbutton;
        [GtkChild]
        private unowned Gtk.ToggleButton day_0;
        [GtkChild]
        private unowned Gtk.ToggleButton day_1;
        [GtkChild]
        private unowned Gtk.ToggleButton day_2;
        [GtkChild]
        private unowned Gtk.ToggleButton day_3;
        [GtkChild]
        private unowned Gtk.ToggleButton day_4;
        [GtkChild]
        private unowned Gtk.ToggleButton day_5;
        [GtkChild]
        private unowned Gtk.ToggleButton day_6;
        [GtkChild]
        private unowned Adw.EntryRow name_entry;
        [GtkChild]
        private unowned Gtk.Revealer mode_revealer;
        [GtkChild]
        private unowned Adw.ComboRow mode;

        private DatabaseConnection dc;
        private string rule_type;
        private static Rule rule;

        construct {
            dc = new DatabaseConnection ();
            rule.selected_days = new uint8[7];
        }

        public RuleSetupDialog (string rule_type) {
            this.rule_type = rule_type;

            this.present ();

            if (this.rule_type == "off") {
                mode_revealer.set_reveal_child (true);
            }
        }

        [GtkCallback]
        private void quit () {
            this.destroy ();
        }

        private void get_values () {
            // Get hour and minutes as integer...
            uint8 hour = (uint8) h_spinbutton.get_value_as_int ();
            uint8 minutes = (uint8) m_spinbutton.get_value_as_int ();
            // ...and convert to string
            rule.hour = "%02d".printf (hour);
            rule.minutes = "%02d".printf (minutes);

            rule.selected_days[0] = (day_0.get_active () == true) ? 1 : 0;
            rule.selected_days[1] = (day_1.get_active () == true) ? 1 : 0;
            rule.selected_days[2] = (day_2.get_active () == true) ? 1 : 0;
            rule.selected_days[3] = (day_3.get_active () == true) ? 1 : 0;
            rule.selected_days[4] = (day_4.get_active () == true) ? 1 : 0;
            rule.selected_days[5] = (day_5.get_active () == true) ? 1 : 0;
            rule.selected_days[6] = (day_6.get_active () == true) ? 1 : 0;

            rule.name = name_entry.get_text ();

            if (rule_type == "off") {
                uint mode_selected = mode.get_selected ();

                switch (mode_selected) {
                case 0:
                    rule.mode = "off";
                    break;
                case 1:
                    rule.mode = "disk";
                    break;
                case 2:
                    rule.mode = "standby";
                    break;
                case 3:
                    rule.mode = "mem";
                    break;
                case 4:
                    rule.mode = "freeze";
                    break;
                }
            }
        }

        // TODO
        // [GtkCallback]
        // private void avoid_duplicated_time () {
        // get_values ();

        // Check if there is another rule at the same time
        // }

        [GtkCallback]
        private bool show_leading_zeros (Gtk.SpinButton spin_button) {
            spin_button.set_text ("%02i".printf (spin_button.get_value_as_int ()));
            return true;
        }

        [GtkCallback]
        private void add_rule_button_clicked () {
            get_values ();

#if PREPROCESSOR_DEBUG
            stdout.printf ("Rule time [HH:MM]: [%s:%s]\n", rule.hour, rule.minutes);
            stdout.printf ("Rule days:\n");
            const string[] days = { "S", "M", "T", "W", "T", "F", "S" };
            for (int i = 0; i < 7; i++) {
                stdout.printf ("\t%s: %d\n", days[i], (int) rule.selected_days[i]);
            }
            stdout.printf ("Rule name: %s\n", rule.name);
            stdout.printf ("Rule type: %s\n", rule_type);
#endif

            string table = (rule_type == "on") ? "rules_turnon" : "rules_turnoff";
            bool success = dc.add_rule (rule, table);

            stdout.printf ("Rule added successfully: %s\n", success ? "true" : "false");

            quit ();
        }
    }
}
