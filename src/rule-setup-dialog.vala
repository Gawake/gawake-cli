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
        private unowned Adw.ComboRow mode;
        [GtkChild]
        private unowned Gtk.Button add_button;

        internal signal void done ();

        private DatabaseConnection dc;
        private string rule_type;
        private static Rule rule;
        private int id;

        construct {
            dc = new DatabaseConnection ();
            rule.selected_days = new uint8[7];
        }

        public RuleSetupDialog.add (string rule_type) {
            // Set title
            title = ("New rule");

            // Set mode Adw.ComboRow visibility
            this.rule_type = rule_type;
            if (this.rule_type == "on") {
                mode.set_visible (false);
            }

            // Connect add button signal to the add function
            add_button.clicked.connect (add_rule);

            this.present ();
        }

        public RuleSetupDialog.edit (string rule_type, int id) {
            this.id = id;

            // Set title
            title = ("Edit rule");

            // Set mode Adw.ComboRow visibility
            this.rule_type = rule_type;
            if (this.rule_type == "on") {
                mode.set_visible (false);
            }

            // Connect add button signal to the add function
            add_button.clicked.connect (edit_rule);

            // Replace window fields with queried data
            rule = dc.query_rule (
                                  (rule_type == "on") ? "rules_turnon" : "rules_turnoff",
                                  id
            );
            h_spinbutton.set_value (double.parse (rule.hour));
            m_spinbutton.set_value (double.parse (rule.minutes));

            day_0.set_active ((bool) rule.selected_days[0]);
            day_1.set_active ((bool) rule.selected_days[1]);
            day_2.set_active ((bool) rule.selected_days[2]);
            day_3.set_active ((bool) rule.selected_days[3]);
            day_4.set_active ((bool) rule.selected_days[4]);
            day_5.set_active ((bool) rule.selected_days[5]);
            day_6.set_active ((bool) rule.selected_days[6]);

            name_entry.set_text (rule.name);

            this.present ();
        }

        private void get_inserted () {
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

        private void add_rule () {
            get_inserted ();

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
            if (success)
                done (); // emit a signal to update view

            quit ();
        }

        private void edit_rule () {
            get_inserted ();

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
            bool success = dc.edit_rule (rule, table, id);

            stdout.printf ("Rule edited successfully: %s\n", success ? "true" : "false");
            if (success)
                done (); // emit a signal to update view

            quit ();
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
        private void quit () {
            this.destroy ();
        }
    } // class RuleSetupDialog
} // namespace Gawake"
