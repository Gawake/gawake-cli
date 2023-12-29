#include "database-connection.h"
#include "gawake.h"

// Getter and setter for the database connection object
static sqlite3 *_db = NULL;

// This function should be called once
gint set_connection (void)
{
  gint ret = connect_database (&_db);
  return ret;
}

sqlite3 *get_connection (void)
{
  return _db;
}

// Connection to the database
gint connect_database (sqlite3 **db)
{
  // Open the SQLite database
  gint rc = sqlite3_open_v2(PATH, db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
    return EXIT_FAILURE;
  }
  else
  {
    gchar *err_msg = 0;
    const gchar *UPTDATE_VERSION = "UPDATE config SET version = '" VERSION "';";
    sqlite3_exec(*db, UPTDATE_VERSION, NULL, 0, &err_msg);
    fprintf(stdout, "Database opened successfully\n");
  }

  return EXIT_SUCCESS;
}

gboolean rule_validated (gRule *rule)
{
  // name can't be bigger than the max value
  gboolean name = !(strlen (rule->name) > RULE_NAME_LENGTH);

  // hour [00,23]
  gboolean hour = (rule->hour >= 0 && rule->hour <= 23);

  // minutes according to enum predefined values
  gboolean minutes = (rule->minutes == M_00
                      || rule->minutes == M_15
                      || rule->minutes == M_30
                      || rule->minutes == M_45);

  // mode according to enum predefined values
  gboolean mode = (rule->mode >= 0 && rule->mode <= OFF);

  gboolean table = (rule->table == T_ON || rule->table == T_OFF);

#if PREPROCESSOR_DEBUG
  g_print ("VALIDATION:\n");
  g_print ("Name: %d\n", name);
  g_print ("\tName length: %ld\n", strlen (rule->name));
  g_print ("Hour: %d\n", hour);
  g_print ("Minutes: %d\n", minutes);
  g_print ("Mode: %d\n", mode);
  g_print ("Table: %d\n\n\n", table);
#endif

  if (name && hour && minutes && mode && table)
    return TRUE;
  else
  {
    g_fprintf (stderr, "Invalid rule.\n");
    return FALSE;
  }
}

gboolean add_rule (const gRule *rule)
{
  gint rc;
  gchar *err_msg = 0;
  gchar *sql = 0;

  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  sqlite3 *db = get_connection ();

  g_snprintf (sql, ALLOC, "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, active) "\
              "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d);",
              rule -> name,
              rule -> hour,
              rule -> minutes,
              rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3], rule -> days[4], rule -> days[5], rule -> days[6],
              rule -> active);

#if PREPROCESSOR_DEBUG
  g_print ("add_rule sql:\n\t%s\n", sql);
#endif

  rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
  if (rc != SQLITE_OK)
    g_fprintf (stderr, "Failed to add rule: %s\n", sqlite3_errmsg(db));

  g_free (sql);
  sqlite3_free (err_msg);

  if (rc == SQLITE_OK)
    return TRUE;
  else
    return FALSE;
}
