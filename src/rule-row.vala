/* rule-row.vala
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

// TODO set translatable fields
// TODO is there a way to optimize memory here?
namespace Gawake {
    [GtkTemplate (ui = "/io/github/kelvinnovais/Gawake/ui/rule-row.ui")]
    private class RuleRow : Gtk.ListBoxRow {
        [GtkChild]
        private unowned Gtk.Switch toggle;
        [GtkChild]
        private unowned Gtk.Label title;
        [GtkChild]
        private unowned Gtk.Label time;
        [GtkChild]
        private unowned Gtk.Label repeats;
        [GtkChild]
        private unowned Gtk.Revealer mode_revealer;
        [GtkChild]
        private unowned Gtk.Label mode;

        private static DatabaseConnection dc;
        static string[] days_abbreviation = { // TODO translators support
                    ("Sun"),
                    ("Mon"),
                    ("Tue"),
                    ("Wed"),
                    ("Thu"),
                    ("Fri"),
                    ("Sat")
        };
        private Gtk.ListBox listbox;

        static construct {
            dc = new DatabaseConnection ();
        }

        public RuleRow (Rule rule, Gtk.ListBox listbox) {
            this.listbox = listbox;

            // Toggle button state
            toggle.set_active (rule.active);
            toggle.state_set.connect (enable_disable);

            // Rule name
            if (rule.name == "") {
                title.set_label (("Unnamed rule"));
            } else {
                title.set_label (rule.name);
            }

            // Time
            time.set_label ("%02d:%02d".printf (rule.hour, rule.minutes));

            // Repeats: ...
            uint8 sum = 0;
            foreach (bool day_state in rule.days) { sum += (uint8) day_state; }
            switch (sum) {
            case 7:
                // ...if all days are set;
                repeats.set_label (("Every Day"));  // TODO translators support
                break;
            case 1:
                // ...if only one day is set
                string[] days_plural = { // TODO translators support
                    ("Sundays"),
                    ("Mondays"),
                    ("Tuesdays"),
                    ("Wednesdays"),
                    ("Thursdays"),
                    ("Fridays"),
                    ("Saturdays")
                };
                for (int i = 0; i < 7; i++) {
                    if (rule.days[i]) {
                        repeats.set_label (days_plural[i]);
                        break;
                    }
                }
                break;
            case 0:
                // ...if any day is set;
                repeats.set_label (("Any Day"));
                break;
            default:
                // ...if only some days are set
                string repeated_days = "";
                for (int i = 0; i < 7; i++) {
                    repeated_days += (rule.days[i]) ? ("%s, ".printf (days_abbreviation[i])) : "";
                }
                // remove the last unnecessary ", "
                repeated_days = repeated_days[0 : -2];

                repeats.set_label (repeated_days);
                break;
            }

            // Mode
            if (rule.table == Table.T_OFF) {
                mode_revealer.set_reveal_child (true);

                switch (rule.mode) {
                case Mode.OFF:
                    mode.set_label ("Off"); // TODO translators support
                    break;
                case Mode.DISK:
                    mode.set_label ("Disk"); // TODO translators support
                    break;
                case Mode.MEM:
                    mode.set_label ("Memory"); // TODO translators support
                    break;
                }
            }

            // Set Gtk element id (numerically equal to the database's id):
            // Format on_XXX for turn on rules
            // Format of_XXX for turn off rules
            this.set_id ("%s%03d".printf ((rule.table == Table.T_ON) ? "on" : "of", rule.id));
        }

        public Gtk.ListBoxRow get_rule_row () {
            return this;
        }

        private Table get_rule_table () {
            // get the Gtk element id, and substring to the rule part
            // if it's eaqual to 'on', convert to rules_turnon;
            // else, to rules_turnoff
            return (this.get_id ().substring (0, 2)) == "on" ? Table.T_ON : Table.T_OFF;
        }

        private int get_rule_id () {
            // similar to previous, but substring it to the number part and convert to int
            return (int.parse (this.get_id ().substring (2, 3)));
        }

        private bool enable_disable () {
            dc.enable_disable_rule (
                                    get_rule_table (),
                                    get_rule_id (),
                                    this.toggle.get_active ()
            );
            return false;
        }

        [GtkCallback]
        private void delete_rule () {
#if PREPROCESSOR_DEBUG
            stdout.printf ("Delete %s rule with ID %d\n",
                           get_rule_table (),
                           get_rule_id ()
            );
#endif

            // If the database deletition was successful, remove row from the view
            if (dc.delete_rule (get_rule_table (), get_rule_id ())) {
                listbox.remove (this);
            } else {
                // TODO notificate
            }
        }
    } // class RuleRow
} // namespace Gawake

