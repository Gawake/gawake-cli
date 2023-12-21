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

namespace Gawake {
    [GtkTemplate (ui = "/com/kelvinnovais/Gawake/ui/window.ui")]
    public class Window : Adw.ApplicationWindow {
        [GtkChild]
        private unowned Adw.ViewStack stack;
        [GtkChild]
        private unowned Gtk.ListBox turnon_listbox;
        [GtkChild]
        private unowned Gtk.ListBox turnoff_listbox;

        private DatabaseConnection dc;

        construct {
            dc = new DatabaseConnection ();
        }

        public Window (Gtk.Application app) {
            Object (application: app);

            load_content ();
        }

        private void load_content () {
            // TODO should it be an async operation?
            // https://wiki.gnome.org/Projects/Vala/Tutorial#Asynchronous_Methods
            int dc_status = dc.init ();
            stdout.printf ("Databases return code: %d%s\n", dc_status, dc_status == 0 ? " (ok)" : " (with error)");

            switch (dc_status) {
            case 0:
                // TODO
                break;
            case -1:
                // TODO
                break;
            case -2:
                dc.load_shared (turnon_listbox, "rules_turnon");
                turnon_listbox.row_activated.connect (edit);
                dc.load_shared (turnoff_listbox, "rules_turnoff");
                turnoff_listbox.row_activated.connect (edit);
                break;
            case -3:
                // TODO
                break;
            }
        }

        public void edit () {
           message ("Edit");
        }

        [GtkCallback]
        void add_rule_clicked () {
            string current_page = stack.get_visible_child_name ();

            new RuleSetupDialog (current_page);
            // current_page is equivalent to rule_type
            // the add action is done on Rule Setup Dialog
        }
    }
}

/* DOCS:
 * Libadwaita: https://gnome.pages.gitlab.gnome.org/libadwaita/doc/1.4/
 */
