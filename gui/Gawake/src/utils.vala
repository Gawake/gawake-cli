namespace Gawake {
// namespace Utils {

// TODO Since this class is extremely simple, can it be compact (?)
// [Compact (opaque=true)]
public class Utils {
    private static string shared_db_dir;
    private static string shared_db_path;
    private static string user;

    // this code will run exactly once (no need to run more than one time)
    static construct {
        user    = Environment.get_variable ("USER");
        shared_db_dir  = "/home/".concat(user, "/Downloads/gawake-tmp/");
        shared_db_path = shared_db_dir.concat ("test.db");
    }

    public string get_user () {
        return user;
    }

    public string get_shared_db_dir () {
        return shared_db_dir;
    }

    public string get_shared_db_path () {
        return shared_db_path;
    }
}

// } // namespace Utils
} // namespace Gawake

// Compact classes: https://lwildberg.pages.gitlab.gnome.org/vala-tutorial/guides/compact-classes.html
