#include "database-connection.h"
#include "gawake.h"

// Getter and setter for the database connection object
static sqlite3 *_db = NULL;

// This function should be called once
int set_connection (void)
{
  int ret = connect_database (&_db);
  return ret;
}

sqlite3 *get_connection (void)
{
  return _db;
}

// Connection to the database
int connect_database (sqlite3 **db)
{
  // Open the SQLite database
  int rc = sqlite3_open_v2(PATH, db, SQLITE_OPEN_READWRITE, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, ANSI_COLOR_RED "Can't open database: %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
    return EXIT_FAILURE;
  }
  else
  {
    char *err_msg = 0;
    const char *UPTDATE_VERSION = "UPDATE config SET version = '" VERSION "';";
    sqlite3_exec(*db, UPTDATE_VERSION, NULL, 0, &err_msg);
    fprintf(stdout, "Database opened successfully!\n");
  }

  return EXIT_SUCCESS;
}

int add_rule (Rule rule)
{
  int alloc = 512; // TODO change
  char *err_msg = 0;
  char *sql = 0;
  sqlite3 *db = get_connection ();

  // TODO check data

  snprintf(sql, alloc, "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, active) "\
           "VALUES ('%s', '%02d%02d00', %d, %d, %d, %d, %d, %d, %d, %d);",
           rule.name,
           rule.hour,
           rule.minutes,
           rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6],
           rule.active);

  return EXIT_SUCCESS;
}
