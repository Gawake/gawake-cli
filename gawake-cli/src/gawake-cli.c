// main file for gawake-cli

/* gawake-cli.c
 *
 * Gawake. A Linux software to make your PC wake up on a scheduled time.
 * It makes the rtcwake command easier.
 *
 * Copyright 2021-2024 Kelvin Novais
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

#include "gawake-cli.h"

gint main (gint argc, gchar **argv)
{
  // Receiving arguments (reference [4])
  gint cflag = 0, mflag = 0, sflag = 0;
  gchar *cvalue = NULL, *mvalue = NULL;
  gint index;
  gint c;

  opterr = 0;

  while ((c = getopt (argc, argv, "hsc:m")) != -1)
    {
      switch (c)
        {
        case 'h':
          usage ();
          return EXIT_SUCCESS;

        case 's':
          sflag = 1;
          break;

        case 'c':
          cflag = 1;
          cvalue = optarg;
          if (strlen (cvalue) != 14)
            {
              g_print ("Invalid time stamp. It must be \"YYYYMMDDhhmmss\".\n");
              return EXIT_FAILURE;
            }
          break;

        case 'm':
          {
            gboolean valid = FALSE;
            mflag = 1;
            mvalue = optarg;
            for (gint i = 0; i < 3; i++)
              {
                if (strcmp (mvalue, MODE[i]) == 0)
                  {
                    valid = TRUE; // If there is a valid mode, continue
                    break;
                  }
              }
            // Exit on invalid mode
            if (!valid)
              {
                g_print ("Invalid mode\n");
                return EXIT_FAILURE;
              }
          }
          break;

        default:
          abort ();
        }
    }

  // Case 's' and/or 'c'; 'm' is optional
  if ((sflag && cflag) || cflag)
    {
      /* TODO call dbus client */
      /* char cmd[FORMATTED_CMD_LEN]; */
      /* snprintf(cmd, FORMATTED_CMD_LEN, "sudo rtcwake -a --date %s", cvalue); */
      if (mflag)
        {
          /* strcat(cmd, " -m "); */
          /* strcat(cmd, mvalue); */
        }
      return EXIT_SUCCESS;
    }
  else if (sflag) // Case 's', only
    {
      if (mflag)
        {
          g_print ("Mode not supported without timestamp (option '-c')\n");
          return EXIT_FAILURE;
        }
      /* TODO call dbus client */
      return EXIT_SUCCESS;
    }

  // Exit if there are invalid arguments
  for (index = optind; index < argc; index++)
    g_print ("Non-option argument \"%s\"\n", argv[index]);
  if (argc > 1)
    return EXIT_FAILURE;

  g_free (cvalue);
  g_free (mvalue);

  // If there's any arguments, continue to the menu
  g_print ("Starting Gawake...\n");

  // Signal handler
  // Triggered on <Ctrl C>
  signal (SIGINT, exit_handler);

  menu ();

  return EXIT_SUCCESS;
}

void menu (void)
{
  gboolean lock = TRUE;
  gchar choice;

  g_print ("---> Choose an option:\n");
  g_print ("[a]\tAdd/remove rules\n"\
           "[s]\tSchedule wake up\n\n"\
           "[c]\tConfigure Gawake\n"\
           "[i]\tInformation about Gawake\n"\
           "[p]\tPrint menu\n"\
           "[q]\tQuit\n");

  // This do-while loop is the menu: receives the user's choice and stops when the 'q' is entered
  do
    {
      g_print ("\n[MENU] ---> ");

      // Receives the user's input
      choice = getchar ();

      if (choice == EOF)
        {
          // When <Ctrl D> was triggered: end execution
          exit_handler (SIGINT);
        }
      else if (choice != '\n' && getchar () != '\n')
        {
          /* Reference [1]
           * A valid answer will have two characters:
           * (1st) a letter in 'choice' and
           * (2nd) a newline on the buffer                   like: ('i', '\n')
           * IF the input is not "empty" ('\n')
           * AND the subsequent character entered is not a newline
           * flush buffered line and invalidate the input
           */
          clear_buffer ();
          choice = 0;
        }
      else
        {
          // Previous conditions passed, just continue to the menu verification
          choice = tolower (choice);
        }

      switch (choice)
        {
        case 'a':
          modify_rule ();
          break;

        case 's':
          g_print (ANSI_COLOR_YELLOW "ATTENTION: Your computer will turn off now\n" ANSI_COLOR_RESET);
          if (confirm ())
            g_print ("todo");  /* schedule (); */
          break;

        case 'c':
          g_print ("todo"); /* TODO config (); */
          break;

        case 'i':
          info ();
          break;

        case 'p':
          g_print ("[a]\tAdd/remove rules\n"\
                   "[s]\tSchedule wake up\n"\
                   "[c]\tConfigure Gawake\n"\
                   "[i]\tInformation about Gawake\n"\
                   "[p]\tPrint menu\n"\
                   "[q]\tQuit\n");
          break;

        case 'q':
          g_print ("Exiting...\n");
          lock = FALSE;
          break;

        case 'k':
          g_print ("Your typo resulted in an easter egg: \n"\
                   "\t\"[...] Porque a minha existência não passa de um elétron, perante o Universo.\"\n");
          break;

        default:
          g_print ("Choose a valid option!\n");
      }
    } while (lock);
}

// Prints information about Gawake
void info (void)
{
  gchar choice[7];

  g_print ("\n[INFORMATION]\n");
  g_print ("gawake-cli version: %s\n", VERSION);
  g_print ("Report issues: <https://github.com/KelvinNovais/Gawake/issues>\n");

  g_print ("Gawake Copyright (C) 2021-2024 Kelvin Ribeiro Novais\n"\
           "This program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\"."\
           "\nThis is free software, and you are welcome to redistribute it under certain conditions; "\
           "type \"show c\" for details.\n\n");

  g_print ("(show w/show c/enter to skip) ---> ");
  fgets (choice, 7, stdin);

  // Checks if the previous string contains a '\n' character at the end;
  // if not, the character is on the buffer and must be cleaned
  if (strchr (choice, '\n') == NULL)
    clear_buffer ();

  if (strcmp (choice, "show w") == 0)
    g_print ("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY"\
             "APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT"\
             "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY"\
             "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,"\
             "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR"\
             "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM"\
             "IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF"\
             "ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
  else if (strcmp(choice, "show c") == 0)
    g_print ("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. "\
             "You also can find the license at <https://www.gnu.org/licenses/>\n\n");
}

// Clears the input buffer
void clear_buffer (void)
{
  gint c;
  while ((c = getchar()) != '\n' && c != EOF) { }
}

// Get the user input, on a valid rule format
void get_user_input (gRule *rule, Table table)
{
  const gchar *DAYS_NAMES[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  gboolean invalid = TRUE;     // This variable is a lock for invalid inputs
  gchar *pch;             // To get the pointer of a new line char, and after, remove it

  // RULE NAME
  // (do-while): repeat if the no name was entered
  g_print ("\n");
  do
    {
      g_print ("Enter the rule name (can't be null) ---> ");
      fgets (rule -> name, RULE_NAME_LENGTH, stdin);

      // Checks if the previous string contains a '\n' character at the end;
      // if not, the character is on the buffvalueer and must be cleaned
      if (strchr (rule -> name, '\n') == NULL)
        clear_buffer ();
    } while (rule -> name[0] == '\n');

  // Removing the new line character
  pch = strstr (rule -> name, "\n");
  if (pch != NULL)
    strncpy (pch, "\0", 1);

  // TIME
  g_print ("\nEnter the time rule will be applied:\n");
  // Hour
  g_print ("%-30s", "[Hour] (from 00 to 23) ");
  gint hour;
  get_int (&hour, 3, 0, 23, 1);
  rule -> hour = (guint8) hour;

  // Minutes
  g_print ("%-30s", "[Minutes] (00, 15, 30 or 45) ");
  invalid = TRUE;
  gint minutes;
  do
    {
      get_int (&minutes, 3, 0, 45, 1);

      switch (minutes)
        {
        case M_00:
          rule -> minutes = M_00;
          invalid = FALSE;
          break;

        case M_15:
          rule -> minutes = M_15;
          invalid = FALSE;
          break;

        case M_30:
          rule -> minutes = M_30;
          invalid = FALSE;
          break;

        case M_45:
          rule -> minutes = M_45;
          invalid = FALSE;
          break;

        default:
          invalid_value ();
        }

    } while (invalid);

  // DAYS
  g_print ("\nEnter the days the rule will be applied (1 for enabled, and 0 for disabled):\n");
  // For each day of the week, receive the user input
  for (gint i = 0; i < 7; i++) {
    g_print ("%-10s", DAYS_NAMES[i]);
    get_int (&(rule -> days[i]), 2, 0, 1, 1);
  }

  // MODE (only for turn off rules)
  if (table == T_OFF) {
    g_print ("\nSelect a mode:\n");

    for (gint i = 0; i < 3; i++)
      g_print ("[%i]\t%s\n", i, MODE[i]);

    gint mode;
    get_int (&mode, 2, 0, 2, 1);
    switch (mode)
      {
      case MEM:
        rule -> mode = MEM;
        break;

      case DISK:
        rule -> mode = DISK;
        break;

      case OFF:
        rule -> mode = OFF;
        break;

      default:
        invalid_value ();
      }
  }

  g_free (DAYS_NAMES);
  g_free (pch);
}

// Tell the user that the entered value was invalid
void invalid_value (void)
{
  g_print (ANSI_COLOR_YELLOW "Please, enter a valid value!\n" ANSI_COLOR_RESET);
}

/*
 * Gets an integer according to:
 * the number of digits wanted;
 * the range (min and max);
 * and if the function must repeat until get a valid value
 *
 * "*ptr" is a pointer to the int variable when the function is called
 *
 * if no value was assigned to variable (passed to this function), and repeat is false
 * when the user enter an invalid input, the variable keeps equal to 0;
 * but if  a previous value was assigned, that value is kept.
 */
void get_int (gint *ptr, gint digits, gint min, gint max, gint repeat)
{
  gchar user_input[digits];    // Number of digits the user input must have
  gint val;
  gboolean invalid = TRUE;
  do
    {
      g_print ("--> ");

      // IF the fgets AND the sscanf are successful...
      if ((fgets(user_input, sizeof(user_input), stdin)) && (sscanf(user_input, "%d", &val) == 1))
        {
          // ...check if the value is in the range and finishes the loop
          if (val >= min && val <= max)
            invalid = FALSE;
        }

      // BUT, IF the user's input is longer than expected, invalidate the value
      if(strchr(user_input, '\n') == NULL)
        {
          if (getchar() != '\n')
            {
              clear_buffer ();
              invalid = TRUE;
            }
        }

      if (invalid && repeat)
        invalid_value (); // Print the invalid message only in case the repeat is required (and the input was invalid)
    } while (invalid && repeat); // Only repeat if the function was called with "repeat = 1"

  *ptr = val;
}

#if 0
// Print Turn on and Turn off tables
int print_table(void __attribute__((__unused__)) *NotUsed,
                int __attribute__((__unused__)) argc,
                char **argv,
                char __attribute__((__unused__)) **azColName) {
  g_print ("%s\n", argv[0]);
  return EXIT_SUCCESS;
}

// Print config table
int print_config(void __attribute__((__unused__)) *NotUsed,
                 int __attribute__((__unused__)) argc,
                 char **argv,
                 char __attribute__((__unused__)) **azColName) {
  NotUsed = 0;
  const char *LABELS[] = {"Gawake status: ", "Commands: ", "Use localtime: ", "rtcwake options: ", "Default mode: ", "For help/more information."};
  int val = 0;
  // Print the 0 and 1 as disabled or enabled, for the 3 first options
  for (int i = 0; i < 3; i++) {
    sscanf(argv[i], "%d", &val);
    if (val == 1)
      g_print ("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_GREEN "Enabled" ANSI_COLOR_RESET);
    else
      g_print ("[%i] %-19s%s\n", i+1, LABELS[i], ANSI_COLOR_YELLOW "Disabled" ANSI_COLOR_RESET);
  }
  // Print the other options
  g_print ("[%i] %-19s%s\n", 4, LABELS[3], argv[3]);
  g_print ("[%i] %-19s%s\n", 5, LABELS[4], argv[4]);
  g_print ("[%i] %-19s\n", 6, LABELS[5]);

  return EXIT_SUCCESS;
}
#endif

#if 0
int config (void) {
  int rc, option = 0, number, alloc = 129;
  struct sqlite3_stmt *stmt;


  char *sql = 0, *string = 0;
  string = (char *) calloc(1, alloc);
  sql = (char *) calloc(1, 2*alloc);
  // Exit if memory allocation fails
  if (sql == NULL || string == NULL) {
    fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: couldn't allocate memory, try again.\n" ANSI_COLOR_RESET);
    return EXIT_FAILURE;
  }

  const char *PRINT_CONFIG = "SELECT status, commands, localtime, options, def_mode FROM config";
  char *err_msg = 0;

  // Print information about time
  // Computer time
  struct tm *timeinfo;
  get_time(&timeinfo);
  g_print ("Time information:\n%-35s %d-%02d-%02d %02d:%02d:%02d\n",
         "Current local time and date:",
         timeinfo -> tm_year + 1900, timeinfo -> tm_mon + 1, timeinfo -> tm_mday, timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec);

  // Database time (localtime and utc)
  rc = sqlite3_prepare_v2(*db, "SELECT datetime('now', 'utc'), datetime('now', 'localtime') LIMIT 1;", -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed gettig database times\n" ANSI_COLOR_RESET);
    return EXIT_FAILURE;
  }
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    char buff[20];
    snprintf(buff, 20, "%s", sqlite3_column_text(stmt, 0));
    g_print ("%-35s %s\n", "Database time (using utc):", buff);
    snprintf(buff, 20, "%s", sqlite3_column_text(stmt, 1));
    g_print ("%-35s %s\n\n", "Database time (using localtime):", buff);
  }
  if (rc != SQLITE_DONE) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed gettig database times (%s)\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
    issue();
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // Get and print the config table
  rc = sqlite3_exec(*db, PRINT_CONFIG, print_config, 0, &err_msg);
  sqlite_exec_err(rc, &err_msg);
  // On success, allow user to change its values
  if (rc == SQLITE_OK) {
    g_print ("To change a value, choose an option (1/2/3/4/5/6/enter to skip) ");
    get_int(&option, 2, 0, 6, 0);

    // If selected option 6 (for help), print it and continue
    if (option == 6) {
      g_print ("\n[HELP]:\n- Gawake status: enable or disable Gawake functionality\n- Commands: the commands you set will only run if this option is enabled\n"\
             "- Use localtime: if disabled, Gawake will use utc instead\n- rtcwake options: you can append the rtcwake command with some other argumments\n"\
             "- Default mode: the mode your computer will sleep when you call \"gawake-cli -c ...\"\n");
      g_print ("To change a value, choose an option (1/2/3/4/5/enter to skip) ");
      option = 0;
      get_int(&option, 2, 0, 5, 0);
    }

    // If the user wants to change a value, continue
    if (option != 0) {
      // The three first options have a common input: get 0 or 1 for them
      if (option < 4) {
        g_print ("Enter the new value (0/1) ");
        get_int(&number, 2, 0, 1, 1);
      }
      switch (option) {
      case 1: // STATUS
        snprintf(sql, alloc, "UPDATE config SET status = %d;", number);
        break;
      case 2: // COMMANDS
        // Print warning and ask confirmation only if commands are going to be enabled
        if (number == 1)
          printf( ANSI_COLOR_YELLOW "WARNING: All commands are executed as root. Keep in mind:\n"\
                  "- It's your responsibility which commands you'll run;\n- Any verification is done;"\
                  "\n- Set the right permissions if you're going to run a script.\n" ANSI_COLOR_RESET);

        if (number == 0 || confirm())
          snprintf(sql, alloc, "UPDATE config SET commands = %d;", number);
        else
          return EXIT_SUCCESS;
        break;
      case 3: // DATABASE TIME
        snprintf(sql, alloc, "UPDATE config SET status = %d;", number);
        break;
      case 4: // RTCWAKE OPTIONS
        // Print warnings, get confirmation; if it's yes, receive the user input (string) and concatenate to the SQL statement
        g_print ( ANSI_COLOR_YELLOW "ATTENTION: BE CAREFUL while changing the options. Check the rtcwake documentation. "\
                "DO NOT append the options with commands. Any verification is done.\n" ANSI_COLOR_RESET);
        if (confirm()) {
          g_print ( "rtcwake manpage: <https://www.man7.org/linux/man-pages/man8/rtcwake.8.html>"\
                  "\nGawake already use \"--date\" and \"-m, --mode\", do not use them.\nGawake default options value: \"-a\"\nEnter the options ---> ");
          strcat(sql, "UPDATE config SET options = '");
          fgets(string, alloc, stdin);
          // Checks if the previous string contains a '\n' character at the end; if not, the character is on the buffer and must be cleaned
          if (strchr(string, '\n') == NULL)
            clear_buffer ();
          char *pch;
          pch = strstr(string, "\n"); // Remove new line character
          if (pch != NULL)
            strncpy(pch, "\0", 1);
          strcat(sql, string);
          strcat(sql, "';");
        }
        break;
      case 5:
        {
          const char *MODES[] = {"off", "disk", "mem", "standby", "freeze", "no", "on", "disable"};
          g_print ("Select a mode:\n");
          for (int i = 0; i < 8; i++)
            g_print ("[%i]\t%s\n", i, MODES[i]);

          get_int(&number, 2, 0, 7, 1);
          snprintf(sql, alloc, "UPDATE config SET def_mode = '%s';", MODES[number]);
        }
        break;
      }

      // Insert data, handle errors
      raise_priv();
      rc = sqlite3_exec(*db, sql, NULL, 0, &err_msg);
      drop_priv();
      if (rc == SQLITE_OK)
        g_print (ANSI_COLOR_GREEN "Successfully done!\n" ANSI_COLOR_RESET);
      sqlite_exec_err(rc, &err_msg);
    }
  }

  // Free allocated memory
  free(sql);
  free(string);
  return EXIT_SUCCESS;
}
#endif

// Add or edit a rule
gint modify_rule (void) {
  /* int rc, option, table, id; */
  gRule rule;
  rule.name = (gchar *) g_malloc (RULE_NAME_LENGTH);
  if (rule.name == NULL)
    {
      g_fprintf (stderr, "Couldn't allocate memory\n");
      return EXIT_FAILURE;
    }

   g_free (rule.name);
   return EXIT_SUCCESS;
}

#if 0
  // SQL statements
  const char *PRINT_TURNON =  "SELECT PRINTF(\"│ %-03i │ %-16.16s│ %s │  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│ %-40.40s│\", "\
                              "id, rule_name, time, sun, mon, tue, wed, thu, fri, sat, command) AS t_on FROM rules_turnon;";
  const char *PRINT_TURNOFF = "SELECT PRINTF(\"│ %-03i │ %-16.16s│ %s │  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│  %-3i│ %-30.30s│ %-8s│\", "\
                              "id, rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) AS t_off FROM rules_turnoff;";

  // Print turn on/off tables, and handle with possible errors:
  // [1] Turn on table
  g_print ( ANSI_COLOR_GREEN "[1] TURN ON RULES\n"\
          "┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────────────────────────────────────────┐"\
          "\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-40s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command");
  rc = sqlite3_exec(*db, PRINT_TURNON, print_table, 0, &err_msg);
  sqlite_exec_err(rc, &err_msg);
  if (rc != SQLITE_OK)
    return EXIT_FAILURE;
  g_print ("└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────────────────────────────────────────┘\n");

  // [2] Turn off table
  g_print ( ANSI_COLOR_YELLOW "[2] TURN OFF RULES\n"\
          "┌─────┬─────────────────┬──────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬───────────────────────────────┬─────────┐"\
          "\n│ %-4s│ %-16s│   Time   │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-30s│ %-8s│\n" ANSI_COLOR_RESET, "ID", "Name", "Command", "Mode");
  rc = sqlite3_exec(*db, PRINT_TURNOFF, print_table, 0, &err_msg);
  sqlite_exec_err(rc, &err_msg);
  if (rc != SQLITE_OK)
    return EXIT_FAILURE;
  g_print ("└─────┴─────────────────┴──────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴───────────────────────────────┴─────────┘\n");

  g_print ("Select a table (1/2/enter to skip) ");
  get_int(&table, 2, 0, 2, 0);
  if (table == 1 || table == 2) {
    int alloc = 512;
    sql = (char *) calloc(1, alloc);
    // Exit if memory allocation fails
    if (sql == NULL) {
      fprintf(stderr, ANSI_COLOR_YELLOW "WARNING: couldn't allocate memory, try again.\n" ANSI_COLOR_RESET);
      return EXIT_FAILURE;
    }
    g_print ("Choose an option:\n[1] Add rule\n[2] Remove rule\n[3] Edit rule\n");
    get_int(&option, 2, 1, 3, 1);
    // If is option 2 or 3, get the ID
    if (option != 1) {
      g_print ("Enter the ID: ");
      get_int(&id, 5, 1, 9999, 1); // ID between 1 and 9999
      if (table == 1)
        snprintf(sql, alloc, "SELECT id FROM rules_turnon WHERE id = %d;", id);
      else
        snprintf(sql, alloc, "SELECT id FROM rules_turnoff WHERE id = %d;", id);

      // Id the ID doesn't exist, leave
      if (  sqlite3_prepare_v2(*db, sql, -1, &selectstmt, NULL) == SQLITE_OK
            && sqlite3_step(selectstmt) != SQLITE_ROW) {
        g_print (ANSI_COLOR_YELLOW "ID not found!\n" ANSI_COLOR_RESET);
        g_print ("Nothing done.\n");
        free(sql);
        sqlite3_finalize(selectstmt);
        return EXIT_FAILURE;
      }
    }
    switch (option) {
    case 1:
      get_user_input(&rule, table);
      if (table == 1) // rules_turnon
        snprintf(sql, alloc,  "INSERT INTO rules_turnon (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command) "\
                 "VALUES ('%s', '%02d:%02d:%02d', %d, %d, %d, %d, %d, %d, %d, '%s');",
                 rule.rule_name,
                 rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6],
                 rule.command);
      else // rules_turnoff
        snprintf(sql, alloc,  "INSERT INTO rules_turnoff (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) "\
                 "VALUES ('%s', '%02d:%02d:%02d', %d, %d, %d, %d, %d, %d, %d, '%s', '%s');",
                 rule.rule_name,
                 rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6],
                 rule.command, rule.mode);
      break;
    case 2:
      if (table == 1) // rules_turnon
        snprintf(sql, alloc, "DELETE FROM rules_turnon WHERE id = %d;", id);
      else // rules_turnoff
        snprintf(sql, alloc, "DELETE FROM rules_turnoff WHERE id = %d;", id);
      break;
    case 3:
      get_user_input(&rule, table);
      if (table == 1) // rules_turnon
        snprintf(sql, alloc, "UPDATE rules_turnon SET rule_name = '%s', time = '%02d:%02d:%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                 "command = '%s' WHERE id = %d;",
                 rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6],
                 rule.command, id);
      else // rules_turnoff
        snprintf(sql, alloc, "UPDATE rules_turnoff SET rule_name = '%s', time = '%02d:%02d:%02d', sun = %d, mon = %d, tue = %d, wed = %d, thu = %d, fri = %d, sat = %d, "\
                 "command = '%s', mode = '%s' WHERE id = %d;",
                 rule.rule_name, rule.time[0], rule.time[1], rule.time[2], rule.days[0], rule.days[1], rule.days[2], rule.days[3], rule.days[4], rule.days[5], rule.days[6],
                 rule.command, rule.mode, id);
      break;
    }
    // Execute SQLite command
    raise_priv();
    rc = sqlite3_exec(*db, sql, NULL, 0, &err_msg);
    drop_priv();
    // Print success or fail
    if (rc == SQLITE_OK)
      g_print (ANSI_COLOR_GREEN "Successfully done!\n" ANSI_COLOR_RESET);
    sqlite_exec_err(rc, &err_msg);

    free(sql);
  }
  return EXIT_SUCCESS;
}
#endif

// Get user's confirmation as 'y'; if confirmed, returns 1
gint confirm (void)
{
  // Reference [1]
  g_print ("Do you want to continue? (y/N) ---> ");
  gchar choice = getchar ();
  if (choice != '\n' && getchar() != '\n')
    {
      // More than one character: flush buffered line and invalidate the input
      clear_buffer ();
      choice = 0 ;
    }
  else
    choice = tolower (choice);

  if (choice == 'y')
    return 1;
  else
    return 0;
}

#if 0
/*
 * DESCRIPTION:
 * The time is gotten in  HHMMSS format, and then converted to integer;
 * Time comparison can simply be done as numbers: e.g. 205500 < 233000.
 * Week days follows struct tm wday: 0 to 6, sun to sat
 */
int schedule(sqlite3 **db) {
  int rc, now, db_time = 1, id_match = -1, alloc = 192;

  struct tm *timeinfo;                        // Default structure, see documentation
  struct sqlite3_stmt *stmt;

  char time[7];
  const char *DB_TIMES[] = {"utc", "localtime"};
  const char *DAYS[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};       // Index(0 to 6) matches tm_wday; the string refers to SQLite column name
  char query[alloc], rtcwake_cmd[FORMATTED_CMD_LEN], buff[7], date[9], mode[8], options[alloc];

  // GET THE DATABASE TIME: 0 FOR UTC AND 1 FOR LOCALTIME
  if (sqlite3_prepare_v2(*db, "SELECT localtime, def_mode, options FROM config WHERE id = 1;", -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed getting config information\n" ANSI_COLOR_RESET);
    issue();
    return EXIT_FAILURE;
  }
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    db_time = sqlite3_column_int(stmt, 0);                                      // from the database: 0 = utc, 1 = localtime
    snprintf(mode, 8, "%s", sqlite3_column_text(stmt, 1));                      // default mode, since this schedule doesn't come from a turn off rule
    snprintf(options, 129, "%s", sqlite3_column_text(stmt, 2));                 // rtcwake options
  }
  if (rc != SQLITE_DONE) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR (failed getting config information): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
    issue();
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // GET THE CURRENT TIME
  get_time(&timeinfo);          // hour, minutes and seconds as integer members
  snprintf(buff, 7, "%02d%02d%02d", timeinfo -> tm_hour, timeinfo -> tm_min, timeinfo -> tm_sec); // Concatenate: HHMMSS as a string
  now = atoi(buff);             // HHMMSS as an integer, leading zeros don't care

  // TRY TO SCHEDULE FOR TODAY
  g_print ("[SCHEDULER] ---> Trying to schedule for today...\n");
  // Create an SQL statement to get today's active rules time; tm_wday = number of the week
  snprintf(query, alloc, "SELECT id, strftime('%%H%%M%%S', time), strftime('%%Y%%m%%d', 'now', '%s') FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC;",
           DB_TIMES[db_time], DAYS[timeinfo -> tm_wday]);
  rc = sqlite3_prepare_v2(*db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for today\n" ANSI_COLOR_RESET);
    issue();
    return EXIT_FAILURE;
  }
  // Get all rules today, ordered by time; the first rule that has a bigger time than now is a valid
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int id              = sqlite3_column_int(stmt, 0);
    int ruletime        = sqlite3_column_int(stmt, 1);
    if (now < ruletime) {
      id_match = id;
      snprintf(date, 9, "%s", sqlite3_column_text(stmt, 2));          // YYYYMMDD
      snprintf(time, 7, "%s", sqlite3_column_text(stmt, 1));          // HHMMSS
      break;
    }
  }
  if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
    fprintf(stderr, ANSI_COLOR_RED "ERROR (failed scheduling for today): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
    issue();
    return EXIT_FAILURE;
  }
  sqlite3_finalize(stmt);

  // IF IT WASN'T POSSIBLE TO SCHEDULE FOR TODAY, TRY ON THE NEXT DAYS
  if (id_match < 0) {
    printf("[SCHEDULER] ---> Any time matched. Trying to schedule for tomorrow or later...\n");
    // search for a matching rule within a week
    for (int i = 1; i <= 7; i++) {
      int wday_num = wday(timeinfo -> tm_wday + i);
      if (wday_num == -1) {
        fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for after (on wday function)\n" ANSI_COLOR_RESET);
        issue();
        return EXIT_FAILURE;
      }
      // The first rule after today is a valid match;
      // also calculate the day: now + number of day until the matching rule, represented by the index i of the loop
      snprintf(query, alloc,  "SELECT id, strftime('%%Y%%m%%d', 'now', '%s', '+%d day'), strftime('%%H%%M%%S', time) FROM rules_turnon WHERE %s = 1 ORDER BY time(time) ASC LIMIT 1;",
                DB_TIMES[db_time], i, DAYS[wday_num]);
      rc = sqlite3_prepare_v2(*db, query, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        fprintf(stderr, ANSI_COLOR_RED "ERROR: Failed scheduling for after\n" ANSI_COLOR_RESET);
        return EXIT_FAILURE;
      }
      while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        id_match = sqlite3_column_int(stmt, 0);
        snprintf(date, 9, "%s", sqlite3_column_text(stmt, 1));        // YYYYMMDD
        snprintf(time, 7, "%s", sqlite3_column_text(stmt, 2));        // HHMMSS
      }
      if (rc != SQLITE_DONE) {
        fprintf(stderr, ANSI_COLOR_RED "ERROR (failed scheduling for after): %s\n" ANSI_COLOR_RESET, sqlite3_errmsg(*db));
        issue();
        return EXIT_FAILURE;
      }
      sqlite3_finalize(stmt);
      if (id_match >= 0) {
        break;
      }
    }
  }

  // IF ANY RULE WAS FOUND, RETURN
  if (id_match < 0) {
    printf(ANSI_COLOR_YELLOW "WARNING: Any turn on rule found. Schedule failed!\n" ANSI_COLOR_RESET);
    return EXIT_SUCCESS;
  }
  // ELSE, SCHEDULE
  sqlite3_close(*db);
  g_print ("Match on turn on rule with ID [%d]\n", id_match);
  snprintf(rtcwake_cmd, FORMATTED_CMD_LEN, "rtcwake --date %s%s %s -m %s", date, time, options, mode);
  g_print (ANSI_COLOR_GREEN "Running rtcwake: %s\n" ANSI_COLOR_RESET, rtcwake_cmd);
  raise_priv();
  system(rtcwake_cmd);
  drop_priv();                // If the command fails, for any reason
  exit(EXIT_SUCCESS);
}
#endif

void usage (void)
{
  g_print ("Gawake (cli version): A Linux software to make your PC wake up on a scheduled time. "\
           "It makes the rtcwake command easier.\n\n"\
           "-c\tSchedule with a custom timestamp (YYYYMMDDhhmmss)\n"\
           "-h\tShow this help and exit\n"\
           "-m\tSet a mode; must be used together '-c'\n"\
           "-s\tDirectly run the schedule function, based on the first upcoming turn on rule; "\
           "you can set a different time with the '-d' option\n"\
           "Examples:\n"\
           "%-45sSchedule according to the next turn on rule\n"\
           "%-45sSchedule wake for 01 January 2025, at 09:45:00\n"\
           "%-45sSchedule wake for 28 December 2025, at 15:30:00; use mode off\n\n",
           "gawake-cli -s", "gawake-cli -c 20250115094500", "gawake-cli -c 20251228153000 -m off");
}

// Close the database and exit on <Ctrl C>
__attribute__((__noreturn__)) void exit_handler (gint sig)
{
  g_print ("\nUser interruption...\n");
  exit (EXIT_FAILURE);
}

// Prints the issue URL and related instructions
void issue (void)
{
  g_print (ANSI_COLOR_RED "If it continues, consider reporting the bug "\
           "<https://github.com/KelvinNovais/Gawake/issues>\n" ANSI_COLOR_RESET);
}

/* REFERENCES:
 * [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
 * [2] https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * [3] https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * [4] https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
 *
 * rtcwake manpage: https://www.man7.org/linux/man-pages/man8/rtcwake.8.html
 */


