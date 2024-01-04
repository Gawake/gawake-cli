#include "database-connection.h"

static sqlite3 *db = NULL;

// Connection to the database
gint connect_database (void)
{
  // Open the SQLite database
  gint rc = sqlite3_open_v2 (PATH, &db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
    {
      g_fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return EXIT_FAILURE;
    }
  else
    {
      gchar *err_msg = 0;

      const gchar UPTDATE_VERSION[] = "UPDATE config SET version = '" VERSION "';";
      rc = sqlite3_exec (db, UPTDATE_VERSION, NULL, 0, &err_msg);
      g_fprintf (stdout, "Database opened successfully\n");

      sqlite3_free (err_msg);
    }

  return EXIT_SUCCESS;
}

void close_database (void)
{
  g_fprintf (stdout, "Closing database\n");

  sqlite3_close (db);
}

static gint validate_rule (const gRule *rule)
{
  // name can't be bigger than the max value
  gboolean name = !(strlen (rule->name) >= RULE_NAME_LENGTH);     // (>=) do not include null terminator

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
    return EXIT_SUCCESS;
  else
    {
      g_fprintf (stderr, "Invalid rule values\n\n");
      return EXIT_FAILURE;
    }
}

static gint validate_table (Table table)
{
  if (table == T_ON || table == T_OFF)
    return EXIT_SUCCESS;

  g_fprintf (stderr, "Invalid table\n\n");
  return EXIT_FAILURE;
}

static gboolean run_sql (const gchar *sql)
{
  gint rc;
  gchar *err_msg = 0;

#if PREPROCESSOR_DEBUG
  g_print ("Generated SQL:\n\t%s\n\n", sql);
#endif

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
  if (validate_rule (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (rule -> table)
    {
    case T_ON:
      g_snprintf (sql, ALLOC, "INSERT INTO rules_turnon "\
                  "(rule_name, time, sun, mon, tue, wed, thu, fri, sat, active) "\
                  "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d);",
                  rule -> name,
                  rule -> hour,
                  rule -> minutes,
                  rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3],
                  rule -> days[4], rule -> days[5], rule -> days[6],
                  rule -> active);
      break;
    case T_OFF:
      g_snprintf (sql, ALLOC, "INSERT INTO rules_turnoff "\
                  "(rule_name, time, sun, mon, tue, wed, thu, fri, sat, active, mode) "\
                  "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d, %d);",
                  rule -> name,
                  rule -> hour,
                  rule -> minutes,
                  rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3],
                  rule -> days[4], rule -> days[5], rule -> days[6],
                  rule -> active,
                  rule -> mode);
      break;
    default:
      g_free (sql);
      return FALSE;
    }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean edit_rule (const gRule *rule)
{
  if (validate_rule (rule))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  switch (rule -> table)
    {
    case T_ON:
      g_snprintf (sql, ALLOC, "UPDATE rules_turnon SET "\
                  "rule_name = '%s', time = '%02d%02d00', "\
                  "sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                  "active = %d WHERE id = %d;",
                  rule -> name,
                  rule -> hour,
                  rule -> minutes,
                  rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3],
                  rule -> days[4], rule -> days[5], rule -> days[6],
                  rule -> active,
                  rule -> id);
      break;
    case T_OFF:
      g_snprintf (sql, ALLOC, "UPDATE rules_turnoff SET "\
                  "rule_name = '%s', time = '%02d%02d00', "\
                  "sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                  "active = %d, mode = %d WHERE id = %d;",
                  rule -> name,
                  rule -> hour,
                  rule -> minutes,
                  rule -> days[0], rule -> days[1], rule -> days[2], rule -> days[3],
                  rule -> days[4], rule -> days[5], rule -> days[6],
                  rule -> active,
                  rule -> mode,
                  rule -> id);
      break;
    default:
      g_free (sql);
      return FALSE;
    }

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean delete_rule (guint16 id, Table table)
{
  if (validate_table (table))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    return FALSE;

  g_snprintf (sql, ALLOC, "DELETE FROM %s WHERE id = %d;", TABLE[table], id);

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean enable_disable_rule (guint16 id, Table table, gboolean active)
{
  if (validate_table (table))
    return FALSE;

  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      g_fprintf (stderr, "Could'n allocate memory\n\n");
      return FALSE;
    }

  g_snprintf (sql, ALLOC, "UPDATE %s SET active = %d WHERE id = %d;", TABLE[table], active, id);

  gboolean ret = run_sql (sql);
  g_free (sql);

  return ret;
}

gboolean query_rule (guint16 id, Table table, gRule *rule)
{
  if (validate_table (table))
    return FALSE;

  // Database related variables
  gint rc;
  struct sqlite3_stmt *stmt;
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Generate SQL
  g_snprintf (sql, ALLOC, "SELECT * FROM %s WHERE id = %d LIMIT 1;", TABLE[table], id);

#if PREPROCESSOR_DEBUG
  g_print ("Generated SQL:\n\t%s\n\n", sql);
#endif

  // Verify ID (also prepare statement)
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      g_fprintf(stderr, "Invalid ID\n\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Query data
  /* ATTENTION columns numbers:
   *    0     1             2       3       (...)       9       10          11
   *    id    rule_name     time    sun     (...)       sat     active      mode
   *                                                                        ^~~~
   *                                                                        |
   *                                                  only for turn off rules
   */
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // ID
      rule -> id = (guint16) sqlite3_column_int (stmt, 0);
      // NAME
      g_snprintf (rule -> name, RULE_NAME_LENGTH, "%s", sqlite3_column_text (stmt, 1));

      // MINUTES AND HOUR
      // TODO sqlite3_column_text16 has correct data type for sscanf, bbut returned data is wrong
      // Temporary variable to receive the minutes and pass to the structure;
      guint8 min;
      sscanf (sqlite3_column_text (stmt, 2), "%02hhd%02hhd", &(rule -> hour), &min);
      rule -> minutes = (Minutes) min;

      // DAYS
      for (gint i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [3,9]
          rule -> days[i] = (gboolean) sqlite3_column_int (stmt, (i+3));
        }

      // ACTIVE
      rule -> active = (gboolean) sqlite3_column_int (stmt, 10); // active

      // MODE (for turn on rules it isn't used):
      rule -> mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 11) : 0);
      rule -> table = table;
    }

  if (rc != SQLITE_DONE)
    {
      g_fprintf(stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg(db));
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  sqlite3_finalize(stmt);
  g_free (sql);

  return TRUE;
}

gboolean query_rules (Table table, gRule **rules, guint16 *rowcount)
{
  if (validate_table (table))
    return FALSE;

  // Database related variables
  gint rc;
  struct sqlite3_stmt *stmt;
  gchar *sql = 0;
  sql = (gchar *) g_malloc (ALLOC);
  if (sql == NULL)
    {
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Count the number of rows
  g_snprintf (sql, ALLOC, "SELECT COUNT(*) FROM %s;", TABLE[table]);
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) == SQLITE_OK
      && sqlite3_step (stmt) != SQLITE_ROW)
    {
      g_fprintf(stderr, "ERROR: Failed to query row count\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }
  *rowcount = sqlite3_column_int (stmt, 0);
#if PREPROCESSOR_DEBUG
  g_print ("Row count: %d\n\n", *rowcount);
#endif

  // Alloc structure
  *rules = malloc (*rowcount * sizeof (**rules));
  if (*rules == NULL)
    {
      g_fprintf(stderr, "ERROR: Failed to allocate memory\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  // Generate SQL
  // SELECT length(<table>.rule_name), * FROM <table>;
  g_snprintf (sql, ALLOC, "SELECT length(%s.rule_name), * FROM %s;", TABLE[table], TABLE[table]);

#if PREPROCESSOR_DEBUG
  g_print ("Generated SQL:\n\t%s\n\n", sql);
#endif

  // Prepare statement
  if (sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
      g_fprintf(stderr, "ERROR: Failed to query rule\n");
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  gint counter = 0;
  // Query data
  /* ATTENTION columns numbers:
   *    0                     1     2             3       4       (...)       10      11        12
   *    rule_name length      id    rule_name     time    sun     (...)       sat     active    mode
   *                                                                                            ^~~~
   *                                                                                            |
   *                                                                      only for turn off rules
   */
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      // If the loop tries to assign on non allocated space, leave
      if (counter > *rowcount)
        break;

      // ID
      (*rules)[counter].id = (guint16) sqlite3_column_int (stmt, 1);

      // NAME
      // Allocate memory
      gint size = sqlite3_column_int (stmt, 0);
      (*rules)[counter].name = (gchar *) g_malloc (size + 1);
      if ((*rules)[counter].name == NULL)
        {
          g_fprintf(stderr, "ERROR: Failed to allocate memory\n");
          return FALSE;
        }

      // Assign name
      g_snprintf ((*rules)[counter].name,         // string pointer
                  size + 1,                       // size
                  "%s",                           // format
                  sqlite3_column_text (stmt, 2)   // arguments
                  );

      // MINUTES AND HOUR
      // TODO sqlite3_column_text16 has correct data type for sscanf, but returned data is wrong
      // Temporary variable to receive the hour and minutes, and then pass to the structure
      guint8 mm, hh;
      sscanf (sqlite3_column_text (stmt, 3), "%02hhd%02hhd", &hh, &mm);
      (*rules)[counter].hour =  hh;
      (*rules)[counter].minutes = (Minutes) mm;

      // DAYS
      for (gint i = 0; i <= 6; i++)
        {
          // days range: [0,6]                  column range: [4,10]
          (*rules)[counter].days[i] = (gboolean) sqlite3_column_int (stmt, (i+4));
        }

      // ACTIVE
      (*rules)[counter].active = (gboolean) sqlite3_column_int (stmt, 11);

      // MODE (for turn on rules it isn't used):
      (*rules)[counter].mode = (Mode) ((table == T_OFF) ? sqlite3_column_int (stmt, 12) : 0);
      (*rules)[counter].table = (guint8) table;

      counter++;
    }

  if (rc != SQLITE_DONE)
    {
      g_fprintf(stderr, "ERROR (failed to query rule): %s\n", sqlite3_errmsg(db));
      g_free (sql);
      sqlite3_finalize (stmt);
      return FALSE;
    }

  sqlite3_finalize(stmt);
  g_free (sql);

  return TRUE;
}

