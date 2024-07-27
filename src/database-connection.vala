/* ! NOT USED ANYMORE !
 * database-connection.vala
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

using Sqlite;

namespace Gawake {

    internal class DatabaseConnection {
        private static Sqlite.Database shared_db;
        private static Sqlite.Statement stmt;
        private static int rc;
        private static string errmsg;
        private static string user;
        private static string shared_db_dir;
        private static string shared_db_path;

        private RuleRow rule_row;

        private static Rule rule;

        static construct {
            Utils utils = new Utils ();

            user = utils.get_user ();
            shared_db_dir = utils.get_shared_db_dir ();
            shared_db_path = utils.get_shared_db_path ();

#if PREPROCESSOR_DEBUG
            stdout.printf ("Getting attributes for database classes:\n");
            stdout.printf ("\tCurrent user: %s\n", user);
            stdout.printf ("\tDatabase directory: %s\n", db_dir);
            stdout.printf ("\tDatabase path: %s\n", db_path);
#endif

            rule.days = new bool[7];
        }

        public bool add_rule (Rule rule) {
            string sql =
                """
                INSERT INTO %s (rule_name, time, sun, mon, tue, wed, thu, fri, sat, active%s)
                VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, 1%s);
                """.printf (
                            TABLE[rule.table],
                            // If the it's the turnoff table, add option "mode" at the end of the columns list...
                            (rule.table == Table.T_OFF) ? (", mode") : "",
                            rule.name,
                            rule.hour,
                            rule.minutes,
                            (int) rule.days[0],
                            (int) rule.days[1],
                            (int) rule.days[2],
                            (int) rule.days[3],
                            (int) rule.days[4],
                            (int) rule.days[5],
                            (int) rule.days[6],
                            (rule.mode == Mode.NONE) ? "" : @", $(rule.mode)"
                );

#if PREPROCESSOR_DEBUG
            stdout.printf ("Add rule SQL:\n%s\n", sql);
#endif
            rc = shared_db.exec (sql, null, out errmsg);
            if (rc != Sqlite.OK) {
                stderr.printf ("Error: %s\n", errmsg);
                return false;
            }

            return true;
        }

        public bool edit_rule (Rule rule, int id) {
            string sql = """
            UPDATE %s SET rule_name = '%s', time = '%02d%02d00', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d,
            active = %d%s WHERE id = %d;
            """.printf (
                        TABLE[rule.table],
                        rule.name,
                        rule.hour,
                        rule.minutes,
                        (int) rule.days[0],
                        (int) rule.days[1],
                        (int) rule.days[2],
                        (int) rule.days[3],
                        (int) rule.days[4],
                        (int) rule.days[5],
                        (int) rule.days[6],
                        rule.active ? 1 : 0,
                        (rule.table == Table.T_OFF) ? @", mode = $(rule.mode)" : "",
                        id
                );

            rc = shared_db.exec (sql, null, out errmsg);
            if (rc != Sqlite.OK) {
                stderr.printf ("Error: %s\n", errmsg);
                return false;
            }
            return true;
        }

        public bool delete_rule (Table table, int id) {
            rc = shared_db.exec (@"DELETE FROM $(TABLE[table]) WHERE id = $id;", null, out errmsg);
            if (rc != Sqlite.OK) {
                stderr.printf ("Error: %s\n", errmsg);
                return false;
            }
            return true;
        }

        public bool enable_disable_rule (Table table, int id, bool state) {
            rc = shared_db.exec (@"UPDATE $(TABLE[table]) SET active = $(state) WHERE id = $id;", null, out errmsg);
            if (rc != Sqlite.OK) {
                stderr.printf ("Error: %s\n", errmsg);
                return false;
            }
            return true;
        }

        public bool load_shared (Gtk.ListBox listbox, Table table) {
            // Prepare statement
            if ((rc = shared_db.prepare_v2 (@"SELECT * FROM $(TABLE[table]);", -1, out stmt, null)) == Sqlite.ERROR) {
                stderr.printf ("[load_shared1] SQL error: %d, %s\n", rc, shared_db.errmsg ());
                return false;
            }
            // Assign to variables
            while ((rc = stmt.step ()) == Sqlite.ROW) {
                rule.id = (uint16) stmt.column_int (0);
                rule.name = stmt.column_text (1);
                rule.hour = (uint8) int.parse (stmt.column_text (2).substring (0, 2));
                rule.minutes = int.parse (stmt.column_text (2).substring (2, 2));
                rule.days[0] = (bool) stmt.column_int (3);
                rule.days[1] = (bool) stmt.column_int (4);
                rule.days[2] = (bool) stmt.column_int (5);
                rule.days[3] = (bool) stmt.column_int (6);
                rule.days[4] = (bool) stmt.column_int (7);
                rule.days[5] = (bool) stmt.column_int (8);
                rule.days[6] = (bool) stmt.column_int (9);
                rule.active = (bool) stmt.column_int (10);
                rule.mode = stmt.column_int (11);

                // create the rule row
                rule_row = new RuleRow (rule, listbox);
                // append to the list box
                listbox.append (rule_row.get_rule_row ());
            }

            // Check if successful
            if (rc != Sqlite.DONE) {
                stderr.printf ("[load_shared2] SQL error: %d, %s\n", rc, shared_db.errmsg ());
                return false;
            }

            return true;
        }

        public Rule query_rule (string table, int id) {
            Rule rule = {
                0,
                0,
                0,
                { false, false, false, false, false, false, false },
                "",
                Mode.OFF,
                true
            };

            // Prepare statement
            if ((rc = shared_db.prepare_v2 (@"SELECT * FROM $table WHERE id = $id;", -1, out stmt, null)) == Sqlite.ERROR) {
                stderr.printf ("[load_shared1] SQL error: %d, %s\n", rc, shared_db.errmsg ());
            }
            // Assign to variables
            while ((rc = stmt.step ()) == Sqlite.ROW) {
                rule.id = (uint16) stmt.column_int (0);
                rule.name = stmt.column_text (1);
                rule.hour = (uint8) int.parse (stmt.column_text (2).substring (0, 2));
                rule.minutes = int.parse (stmt.column_text (2).substring (2, 2));
                rule.days[0] = (bool) stmt.column_int (3);
                rule.days[1] = (bool) stmt.column_int (4);
                rule.days[2] = (bool) stmt.column_int (5);
                rule.days[3] = (bool) stmt.column_int (6);
                rule.days[4] = (bool) stmt.column_int (7);
                rule.days[5] = (bool) stmt.column_int (8);
                rule.days[6] = (bool) stmt.column_int (9);
                rule.active = (bool) stmt.column_int (10);
                rule.mode = stmt.column_int (11);
            }
            // Check if successful
            if (rc != Sqlite.DONE) {
                stderr.printf ("[load_shared2] SQL error: %d, %s\n", rc, shared_db.errmsg ());
            }

            return rule;
        }

        public bool init_shared_db () {
            // Check if the directory exists
            // TODO (non-default base directory locations): https://docs.flatpak.org/en/latest/sandbox-permissions.html
            // (!) Can not access /var or other directories outside the sand box
            File directory = File.new_for_path (shared_db_dir);
            FileType d_exists = directory.query_file_type (FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
            if (d_exists != GLib.FileType.DIRECTORY) {
                stdout.printf ("Shared database directory doesn't exist, creating it...\n");
                // FIXME group is set as 5, not as 7
                DirUtils.create_with_parents (shared_db_dir, 0775);
            }

            // Check if the database exists
            File file = File.new_for_path (shared_db_path);
            bool f_exists = file.query_exists ();
            stdout.printf ("Shared database exists: %s\n", f_exists ? "true" : "false");

            rc = Sqlite.Database.open (shared_db_path, out shared_db);
            if (rc != Sqlite.OK) {
                stderr.printf ("Can't open shared database: %d: %s\n", shared_db.errcode (), shared_db.errmsg ());
                return false;
            }

            if (!f_exists) {
                stdout.printf ("Configuring new shared database...\n");
                const string SQL_CREATE =
                    """CREATE TABLE rules_turnon (
                      id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                      rule_name   TEXT NOT NULL,
                      time        TEXT NOT NULL,
                      sun         INTEGER NOT NULL,
                      mon         INTEGER NOT NULL,
                      tue         INTEGER NOT NULL,
                      wed         INTEGER NOT NULL,
                      thu         INTEGER NOT NULL,
                      fri         INTEGER NOT NULL,
                      sat         INTEGER NOT NULL,
                      active      INTEGER NOT NULL
                  );
                  CREATE TABLE rules_turnoff (
                      id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                      rule_name   TEXT NOT NULL,
                      time        TEXT NOT NULL,
                      sun         INTEGER NOT NULL,
                      mon         INTEGER NOT NULL,
                      tue         INTEGER NOT NULL,
                      wed         INTEGER NOT NULL,
                      thu         INTEGER NOT NULL,
                      fri         INTEGER NOT NULL,
                      sat         INTEGER NOT NULL,
                      active      INTEGER NOT NULL,
                      mode        TEXT NOT NULL
                  );
                  CREATE TABLE config (
                      id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                      options     TEXT,
                      status      INTEGER NOT NULL,
                      version     TEXT,
                      commands    INTEGER NOT NULL,
                      localtime   INTEGER NOT NULL,
                      def_mode    TEXT NOT NULL,
                      boot_time   INTEGER NOT NULL
                  );
                  INSERT INTO config (options, status, version, commands, localtime, def_mode, boot_time)
                  VALUES ('-a', 1, '1.0.0', 0, 1, 'off', 120);""";
                // TODO:            ^^^^^ replace version with variable

                rc = shared_db.exec (SQL_CREATE, null, out errmsg);
                if (rc != Sqlite.OK) {
                    stderr.printf ("Error: %s\n", errmsg);
                    return false;
                }
            }

            return true;
        }
    } // DatabaseConnection
} // namespace Gawake

/* DOCS.:
 * https://vala-language.org/vala-docs/sqlite3/Sqlite.Database.html
 * https://wiki.gnome.org/Projects/Vala/SqliteSample
 */

