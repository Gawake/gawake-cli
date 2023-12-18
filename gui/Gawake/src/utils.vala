namespace Gawake {
// namespace Utils {

// Since this class is extremely simple, it can be compact
[Compact (opaque=true)]
public class Utils {
    private static string db_dir;
    private static string db_path;
    private static string user;

    // this code will run exactly once (no need to run more than one time)
    /* static construct {
        // TODO treat root user
        user = Environment.get_variable ("USER");
        db_path = "/home/".concat(user, "/.config/gawake/");
    } */

    public Utils () {
        user    = Environment.get_variable ("USER");
        db_dir  = "/home/".concat(user, "/Downloads/gawake-tmp/");
        db_path = db_dir.concat ("test.db");
    }

    public string get_user () {
        return user;
    }

    public string get_db_dir () {
        return db_dir;
    }

    public string get_db_path () {
        return db_path;
    }
}

// } // namespace Utils
} // namespace Gawake

// Compact classes: https://lwildberg.pages.gitlab.gnome.org/vala-tutorial/guides/compact-classes.html
