using GLib; // TODO do I need to use this explicitly?
using Sqlite;

namespace Gawake {
// namespace DatabaseConnection {

// TODO should this class be public?
// try internal
public class DatabaseConnection : GLib.Object {
    private static Sqlite.Database db;
    // private Sqlite.Statement stmt;
    private static int rc;
    private static string errmsg;
    private static string user;
    private static string db_dir;
    private static string db_path;

     public DatabaseConnection () {
        Utils utils = new Utils ();

        user    = utils.get_user();
        db_dir  = utils.get_db_dir();
        db_path = utils.get_db_path();

        db_dir  = "/home/kelvin/Downloads/gawake-tmp";
        db_path = "/home/kelvin/Downloads/gawake-tmp/test.db";

#if PREPROCESSOR_DEBUG
	    stdout.printf ("User: %s\n", user);
	    stdout.printf ("Database directory: %s\n", db_dir);
	    stdout.printf ("Database path: %s\n", db_path);
#endif
    }

    /* public static int callback (int n_columns, string[] values,
                                string[] column_
                                names)
    {
        for (int i = 0; i < n_columns; i++) {
            stdout.printf ("%s = %s\n", column_names[i], values[i]);
        }
        stdout.printf ("\n");

        return 0;
    } */

    public static int init () {
        // TODO
        // Check for the database file existence;
        // Handle with possible errors (database does not exist);
        // Open the database connection.

        // Check if the directory exists
        // TODO (non-default base directory locations): https://docs.flatpak.org/en/latest/sandbox-permissions.html
        File directory = File.new_for_path (db_dir);
        FileType d_exists = directory.query_file_type (FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
        if (d_exists != GLib.FileType.DIRECTORY) {
            stdout.printf ("Database directory doesn't exist, creating it...\n");
            // FIXME group is set as 5, not as 7
            DirUtils.create_with_parents (db_dir, 0775);
        }

        // Check if the database exists
        File file = File.new_for_path (db_path);
        bool f_exists = file.query_exists ();
        //print (@"file exists: $f_exists\n");

        rc = Sqlite.Database.open (db_path, out db);
        if (rc != Sqlite.OK) {
		    stderr.printf ("Can't open database: %d: %s\n", db.errcode (), db.errmsg ());
		    return -1;
	    }

	    if (!f_exists) {
            stdout.printf ("Database doesn't exist, creating.\n");
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
                      sat         INTEGER NOT NULL
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
                  INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat)
                  VALUES ('Example', '10:00:00', 0, 0, 0, 0, 0, 0, 0);
                  INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, mode)
                  VALUES ('Example', '11:30:00', 0, 0, 0, 0, 0, 0, 0, 'mem');
                  INSERT INTO config (options, status, version, commands, localtime, def_mode, boot_time)
                  VALUES ('-a', 1, '1.0.0', 0, 1, 'off', 300);""";
            // TODO:                ^^^^^ replace version with variable

            rc = db.exec (SQL_CREATE, null, out errmsg);
	        if (rc != Sqlite.OK) {
		        stderr.printf ("Error: %s\n", errmsg);
		        return -1;
	        }
        }

        // Open a database:


        // rc = db.exec (args[2], callback, null);
        /* maybe it is better to use closures, so you can access local variables, eg: */
        /*rc = db.exec(args[2], (n_columns, values, column_names) => {
            for (int i = 0; i < n_columns; i++) {
                stdout.printf ("%s = %s\n", column_names[i], values[i]);
            }
            stdout.printf ("\n");

            return 0;
            }, null);
        */

        return 0;
    }
}

// } // namespace DatabaseConnection
} // namespace Gawake

/* DOCS.:
 * https://vala-language.org/vala-docs/sqlite3/Sqlite.Database.html
 * https://wiki.gnome.org/Projects/Vala/SqliteSample
 */



