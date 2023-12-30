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
    [GtkTemplate (ui = "/io/github/kelvinnovais/Gawake/ui/rule-setup-dialog.ui")]
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
        private unowned Gtk.Button action_button;

        internal signal void done ();

        private DatabaseConnection dc;
        private static Rule rule;
        private int id;

        construct {
            dc = new DatabaseConnection ();
            rule.days = new bool[7];
        }

        public RuleSetupDialog.add (Table table) {
            // Set title and button label
            title = (("New rule")); // TODO translate support
            action_button.set_label (("Add")); // TODO translate support

            // Set mode Adw.ComboRow visibility
            rule.table = table;
            if (rule.table == Table.T_ON) {
                mode.set_visible (false);
            }

            // Connect add button signal to the add function
            action_button.clicked.connect (add_rule);

            this.present ();
        }

        public RuleSetupDialog.edit (Table table, int id) {
            this.id = id;

            // Set title and button label
            title = (("Edit rule")); // TODO translate support
            action_button.set_label (("Done")); // TODO translate support

            // Set mode Adw.ComboRow visibility
            rule.table = table;
            if (rule.table == Table.T_ON) {
                mode.set_visible (false);
            }

            // Connect add button signal to the add function
            action_button.clicked.connect (edit_rule);

            // Replace window fields with queried data
            rule = dc.query_rule (
                                  (table == T_ON) ? "rules_turnon" : "rules_turnoff",
                                  id
            );
            h_spinbutton.set_value (rule.hour);
            m_spinbutton.set_value ((double) rule.minutes);

            day_0.set_active (rule.days[0]);
            day_1.set_active (rule.days[1]);
            day_2.set_active (rule.days[2]);
            day_3.set_active (rule.days[3]);
            day_4.set_active (rule.days[4]);
            day_5.set_active (rule.days[5]);
            day_6.set_active (rule.days[6]);

            name_entry.set_text (rule.name);

            this.present ();
        }

        // Get user input values from the Dialog
        private void get_inserted () {
            // Get hour and minutes
            rule.hour = (uint8) h_spinbutton.get_value_as_int ();
            rule.minutes = (uint8) m_spinbutton.get_value_as_int ();

            rule.days[0] = day_0.get_active ();
            rule.days[1] = day_1.get_active ();
            rule.days[2] = day_2.get_active ();
            rule.days[3] = day_3.get_active ();
            rule.days[4] = day_4.get_active ();
            rule.days[5] = day_5.get_active ();
            rule.days[6] = day_6.get_active ();

            rule.name = name_entry.get_text ();

            if (rule.table == Table.T_OFF) {
                rule.mode = mode.get_selected ();
            }
        }

        private void add_rule () {
            get_inserted ();

#if PREPROCESSOR_DEBUG
            stdout.printf ("Rule time [HH:MM]: [%s:%s]\n", rule.hour, rule.minutes);
            stdout.printf ("Rule days:\n");
            const string[] days = { "S", "M", "T", "W", "T", "F", "S" };
            for (int i = 0; i < 7; i++) {
                stdout.printf ("\t%s: %d\n", days[i], (int) rule.days[i]);
            }
            stdout.printf ("Rule name: %s\n", rule.name);
            stdout.printf ("Rule type: %s\n", rule_type);
#endif

            bool success = dc.add_rule (rule);

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
                stdout.printf ("\t%s: %d\n", days[i], (int) rule.days[i]);
            }
            stdout.printf ("Rule name: %s\n", rule.name);
            stdout.printf ("Rule type: %s\n", rule_type);
#endif

            bool success = dc.edit_rule (rule, id);

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

