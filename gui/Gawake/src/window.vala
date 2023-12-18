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
    [GtkTemplate (ui = "/com/kelvinnovais/Gawake/window.ui")]
    public class Window : Adw.ApplicationWindow {
        [GtkChild]
        private unowned Gtk.Button add_rule;
        [GtkChild]
        private unowned Adw.ViewStack stack;

        public Window (Gtk.Application app) {
            Object (application: app);
        }

        construct {
            add_rule.clicked.connect (add_rule_clicked);
        }

        void add_rule_clicked () {
            string? current_page = stack.get_visible_child_name ();
            stdout.printf ("Add rule button clicked\n");
            stdout.printf ("Current page: %s\n", current_page);
        }
    }
}

/* DOCS:
 * Libadwaita: https://gnome.pages.gitlab.gnome.org/libadwaita/doc/1.4/
 */
