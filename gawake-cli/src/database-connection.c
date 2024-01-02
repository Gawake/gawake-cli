#include "database-connection.h"

// Getter and setter for the database connection object
static sqlite3 *_db = NULL;

// Setter
// This function should be called once
gint set_connection (void)
{
  gint ret = connect_database (&_db);
  return ret;
}

// Getter
sqlite3 *get_connection (void)
{
  return _db;
}

// Connection to the database
gint connect_database (sqlite3 **db)
{
  // Open the SQLite database
  gint rc = sqlite3_open_v2 (PATH, db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (*db));
    sqlite3_close (*db);
    return EXIT_FAILURE;
  }
  else
  {
    gchar *err_msg = 0;

    const gchar UPTDATE_VERSION[] = "UPDATE config SET version = '" VERSION "';";
    rc = sqlite3_exec (*db, UPTDATE_VERSION, NULL, 0, &err_msg);
    fprintf (stdout, "Database opened successfully\n");

    sqlite3_free (err_msg);
  }

  return EXIT_SUCCESS;
}

void close_database (void)
{
  fprintf (stdout, "Closing database\n");
  sqlite3 *db = get_connection ();

  sqlite3_close (db);
}

static gboolean rule_validated (gRule *rule)
{
  // name can't be bigger than the max value
  gboolean name = !(strlen (rule->name) > RULE_NAME_LENGTH);

  // hour [00,23]
  gboolean hour = (rule->hour <= 23);     // Due to the data type, houver is always >= 0

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
  g_print ("Table: %d\n\n", table);
#endif

  if (name && hour && minutes && mode && table)
    return TRUE;
  else
  {
    g_fprintf (stderr, "Invalid rule values\n\n");
    return FALSE;
  }
}

static gboolean run_sql (const gchar *sql)
{
  gint rc;
  gchar *err_msg = 0;

#if PREPROCESSOR_DEBUG
  g_print ("Generated SQL:\n\t%s\n", sql);
#endif

  sqlite3 *db = get_connection ();

  rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);
  if (rc != SQLITE_OK)
    g_fprintf (stderr, "Failed to run SQL: %s\n", sqlite3_errmsg(db));

  sqlite3_free (err_msg);

  if (rc == SQLITE_OK)
    return TRUE;
  else
    return FALSE;
}

gboolean add_rule (const gRule *rule)
{
  if (!rule_validated (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (rule -> table)
  {
  case T_ON:
    g_snprintf (sql, ALLOC, "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, active) "\
                "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d);",
                rule -> name,
                rule -> hour,
                rule -> minutes,
                rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3], rule -> days[4], rule -> days[5], rule -> days[6],
                rule -> active);
    break;
  case T_OFF:
    g_snprintf (sql, ALLOC, "INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, active, mode) "\
                "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d, %d);",
                rule -> name,
                rule -> hour,
                rule -> minutes,
                rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3], rule -> days[4], rule -> days[5], rule -> days[6],
                rule -> active,
                rule -> mode);
    break;
  default:
    return FALSE;
  }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean edit_rule (const gRule *rule)
{
  if (!rule_validated (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (rule -> table)
  {
  case T_ON:
    g_snprintf (sql, ALLOC, "UPDATE rules_turnon SET rule_name = '%s', time = '%02d%02d00', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                "active = %d WHERE id = %d;",
                rule -> name,
                rule -> hour,
                rule -> minutes,
                rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3], rule -> days[4], rule -> days[5], rule -> days[6],
                rule -> active,
                rule -> id);
    break;
  case T_OFF:
    g_snprintf (sql, ALLOC, "UPDATE rules_turnoff SET rule_name = '%s', time = '%02d%02d00', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                "active = %d, mode = %d WHERE id = %d;",
                rule -> name,
                rule -> hour,
                rule -> minutes,
                rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3], rule -> days[4], rule -> days[5], rule -> days[6],
                rule -> active,
                rule -> mode,
                rule -> id);
    break;
  default:
    return FALSE;
  }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean delete_rule (guint16 id, guint8 table)
{
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (table)
  {
  case T_ON:
    g_snprintf (sql, ALLOC, "DELETE FROM rules_turnon WHERE id = %d;", id);
    break;
  case T_OFF:
    g_snprintf (sql, ALLOC, "DELETE FROM rules_turnoff WHERE id = %d;", id);
    break;
  default:
    return FALSE;
  }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean enable_disable_rule (guint16 id, guint8 table, gboolean active)
{
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (table)
  {
  case T_ON:
    g_snprintf (sql, ALLOC, "UPDATE rules_turnon SET active = %d WHERE id = %d;", active, id);
    break;
  case T_OFF:
    g_snprintf (sql, ALLOC, "UPDATE rules_turnoff SET active = %d WHERE id = %d;", active, id);
    break;
  default:
    return FALSE;
  }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}
