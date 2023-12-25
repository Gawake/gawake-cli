/* window.vala
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

// TODO see https://www.vala-project.org/doc/vala/Methods.html#Contract_programming

namespace Gawake {
    [GtkTemplate (ui = "/io/github/kelvinnovais/Gawake/ui/window.ui")]
    public class Window : Adw.ApplicationWindow {
        [GtkChild]
        private unowned Adw.ViewStack stack;
        [GtkChild]
        private unowned Gtk.ListBox turnon_listbox;
        [GtkChild]
        private unowned Gtk.ListBox turnoff_listbox;

        private DatabaseConnection dc;
        private static bool shared_db_status = false;
        private static bool user_db_status = false;

        construct {
            dc = new DatabaseConnection ();

            turnon_listbox.row_activated.connect ((rule_row) => {
                RuleSetupDialog rsd = new RuleSetupDialog.edit (
                                                                "on",
                                                                int.parse (rule_row.get_id ().substring (2, 3))
                );
                rsd.done.connect (update);
            });

            turnoff_listbox.row_activated.connect ((rule_row) => {
                RuleSetupDialog rsd = new RuleSetupDialog.edit (
                                                                "off",
                                                                int.parse (rule_row.get_id ().substring (2, 3))
                );
                rsd.done.connect (update);
            });
        }

        public Window (Gtk.Application app) {
            Object (application: app);

            connect_dbs ();
        }

        private void connect_dbs () {
            // TODO should it be an async operation?
            // https://wiki.gnome.org/Projects/Vala/Tutorial#Asynchronous_Methods

            stdout.printf ("Opening databases...\n");

            if (!shared_db_status)
                shared_db_status = dc.init_shared_db ();

            // TODO
            // if (user_db_status)
            // user_db_status = dc.init_user_db ();


            stdout.printf ("Databases return code:\n\tShared: %d\n\tUser: %d\n", (int) shared_db_status, (int) user_db_status);

            load_content ();
        }

        private void load_content () {
            if (shared_db_status) {
                // clean rows if exist
                turnon_listbox.remove_all ();
                turnoff_listbox.remove_all ();

                dc.load_shared (turnon_listbox, "rules_turnon");
                dc.load_shared (turnoff_listbox, "rules_turnoff");
            }
        }

        public void update () {
            if (shared_db_status) {
                // clean rows if exist
                turnon_listbox.remove_all ();
                turnoff_listbox.remove_all ();

                dc.load_shared (turnon_listbox, "rules_turnon");
                dc.load_shared (turnoff_listbox, "rules_turnoff");
            }
        }

        [GtkCallback]
        void add_button_clicked () {
            string current_page = stack.get_visible_child_name ();

            // current_page is equivalent to rule_type
            // the add action is done on Rule Setup Dialog class
            RuleSetupDialog rsd = new RuleSetupDialog.add (current_page);
            rsd.done.connect (update);
        }
    }
}

/* DOCS:
 * Libadwaita: https://gnome.pages.gitlab.gnome.org/libadwaita/doc/1.4/
 */
