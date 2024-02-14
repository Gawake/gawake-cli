/* main.c
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

// main file for gawake-cli

#include "main.h"
#include "dbus-client.h"

gint main (gint argc, gchar **argv)
{
  // If can't connect to the D-Bus client, exit
  if (connect_dbus_client ())
    return EXIT_FAILURE;

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
              printf ("Invalid time stamp. It must be \"YYYYMMDDhhmmss\".\n");
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
                printf ("Invalid mode\n");
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
          printf ("Mode is only supported with a timestamp (option '-c')\n");
          return EXIT_FAILURE;
        }
      /* TODO call dbus client */
      return EXIT_SUCCESS;
    }

  // Exit if there are invalid arguments
  for (index = optind; index < argc; index++)
    printf ("Non-option argument \"%s\"\n", argv[index]);
  if (argc > 1)
    return EXIT_FAILURE;

  g_free (cvalue);
  g_free (mvalue);

  // If there's any arguments, continue to the menu
  printf ("Starting Gawake...\n");

  // Signal handler
  // Triggered on <Ctrl C>
  signal (SIGINT, exit_handler);

  menu ();

  // Close D-Bus connection
  close_dbus_client ();

  return EXIT_SUCCESS;
}

void menu (void)
{
  gboolean lock = TRUE;
  gchar choice;

  printf ("---> Choose an option:\n");
  printf ("[a]\tAdd/remove rules\n"\
          "[s]\tSchedule wake up\n"\
          "[c]\tConfigure Gawake\n"\
          "[i]\tInformation about Gawake\n"\
          "[p]\tPrint menu\n"\
          "[q]\tQuit\n");

  // This do-while loop is the menu: receives the user's choice and stops when the 'q' is entered
  do
    {
      printf ("\n[MENU] ---> ");

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
          add_remove_rule ();
          break;

        case 's':
          printf (YELLOW ("ATTENTION: Your computer will turn off now\n"));
          if (confirm ())
            printf ("todo");  /* schedule (); */
          break;

        case 'c':
          printf ("todo"); /* TODO config (); */
          break;

        case 'i':
          info ();
          break;

        case 'p':
          printf ("[a]\tAdd/remove rules\n"\
                  "[s]\tSchedule wake up\n"\
                  "[c]\tConfigure Gawake\n"\
                  "[i]\tInformation about Gawake\n"\
                  "[p]\tPrint menu\n"\
                  "[q]\tQuit\n");
          break;

        case 'q':
          printf ("Exiting...\n");
          lock = FALSE;
          break;

        case 'k':
          printf ("Your typo resulted in an easter egg: \n"\
                  "\t\"[...] Porque a minha existência não passa de um elétron, perante o Universo.\"\n");
          break;

        default:
          printf ("Choose a valid option!\n");
      }
    } while (lock);
}

// Prints information about Gawake
void info (void)
{
  gchar choice[7];

  printf ("\n[INFORMATION]\n");
  printf ("gawake-cli version: %s\n", VERSION);
  printf ("Report issues: <https://github.com/KelvinNovais/Gawake/issues>\n");

  printf ("Gawake Copyright (C) 2021-2024 Kelvin Ribeiro Novais\n"\
          "This program comes with ABSOLUTELY NO WARRANTY; for details type \"show w\"."\
          "\nThis is free software, and you are welcome to redistribute it under certain conditions; "\
          "type \"show c\" for details.\n\n");

  printf ("(show w/show c/enter to skip) ---> ");
  fgets (choice, 7, stdin);

  // Checks if the previous string contains a '\n' character at the end;
  // if not, the character is on the buffer and must be cleaned
  if (strchr (choice, '\n') == NULL)
    clear_buffer ();

  if (strcmp (choice, "show w") == 0)
    printf ("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY"\
            "APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT"\
            "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY"\
            "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,"\
            "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR"\
            "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM"\
            "IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF"\
            "ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
  else if (strcmp(choice, "show c") == 0)
    printf ("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. "\
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
  const gchar DAYS_NAMES[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  gboolean invalid = TRUE;     // This variable is a lock for invalid inputs
  gchar *pch;                  // To get the pointer of a new line char, and after, remove it

  // NAME
  // (do-while): repeat if the no name was entered
  printf ("\n");
  do
    {
      printf ("Enter the rule name (can't be null) ---> ");
      fgets (rule->name, RULE_NAME_LENGTH, stdin);

      // Exit if user enters Ctrl D
      if (feof (stdin))
        exit_handler (SIGINT);

      // Checks if the previous string contains a '\n' character at the end;
      // if not, the character is on the buffvalueer and must be cleaned
      if (strchr (rule->name, '\n') == NULL)
        clear_buffer ();
    } while (rule->name[0] == '\n');

  // Removing the new line character
  pch = strstr (rule->name, "\n");
  if (pch != NULL)
    strncpy (pch, "\0", 1);

  // TIME
  printf ("\nEnter the time rule will be applied:\n");
  // Hour
  printf ("%-30s", "[Hour] (from 00 to 23) ");
  gint hour;
  get_int (&hour, 3, 0, 23, 1);
  rule->hour = (guint8) hour;

  // Minutes
  printf ("%-30s", "[Minutes] (00, 15, 30 or 45) ");
  invalid = TRUE;
  gint minutes;
  do
    {
      get_int (&minutes, 3, 0, 45, 1);

      switch (minutes)
        {
        case M_00:
          rule->minutes = M_00;
          invalid = FALSE;
          break;

        case M_10:
          rule->minutes = M_10;
          invalid = FALSE;
          break;

        case M_20:
          rule->minutes = M_20;
          invalid = FALSE;
          break;

        case M_30:
          rule->minutes = M_30;
          invalid = FALSE;
          break;

        case M_40:
          rule->minutes = M_40;
          invalid = FALSE;
          break;

        case M_50:
          rule->minutes = M_50;
          invalid = FALSE;
          break;

        default:
          invalid_value ();
        }

    } while (invalid);

  // DAYS
  printf ("\nEnter the days the rule will be applied (1 for enabled, and 0 for disabled):\n");
  // For each day of the week, receive the user input
  for (gint i = 0; i < 7; i++) {
    printf ("%-10s", DAYS_NAMES[i]);
    get_int (&(rule->days[i]), 2, 0, 1, 1);
  }

  // ACTIVE
  rule->active = TRUE; // when adding a rule, it's always active

  // MODE (only for turn off rules)
  if (table == T_OFF)
    {
      printf ("\nSelect a mode:\n");

      for (gint i = 0; i < 3; i++)
        printf ("[%i]\t%s\n", i, MODE[i]);

      gint mode;
      get_int (&mode, 2, 0, 2, 1);
      switch (mode)
        {
        case MEM:
          rule->mode = MEM;
          break;

        case DISK:
          rule->mode = DISK;
          break;

        case OFF:
          rule->mode = OFF;
          break;

        default:
          invalid_value ();
        }
    }
  else
    {
       // Just pass a valid value
       rule->mode = MEM;
    }

  // TABLE
  rule->table = table;

  pch = NULL;
}

// Tell the user that the entered value was invalid
void invalid_value (void)
{
  printf (YELLOW ("Please, enter a valid value!\n"));
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
      printf ("--> ");

      // If the fgets AND the sscanf are successful...
      if ((fgets (user_input, sizeof (user_input), stdin))
          && (sscanf (user_input, "%d", &val) == 1))
        {
          // ...check if the value is in the range and finishes the loop
          if (val >= min && val <= max)
            invalid = FALSE;
        }

      // Exit if user enters Ctrl D
      if (feof (stdin))
        exit_handler (SIGINT);

      // If the user's input is longer than expected, invalidate the value
      if(strchr (user_input, '\n') == NULL)
        {
          if (getchar () != '\n')
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

// Add or remove a rule
gint add_remove_rule (void) {
  gint table, action, id;
  gRule rule;

  rule.name = (gchar *) g_malloc (RULE_NAME_LENGTH);
  if (rule.name == NULL)
    {
      fprintf (stderr, RED ("Couldn't allocate memory\n"));
      return EXIT_FAILURE;
    }

  if (query_rules (T_ON))
    return EXIT_FAILURE;

  if (query_rules (T_OFF))
    return EXIT_FAILURE;

  printf ("Select a table (1/2) ");
  get_int (&table, 2, 1, 2, 0);
  table -= 1;
  if (table != T_ON && table != T_OFF)
    {
      printf ("Invalid table\n");
      return EXIT_FAILURE;
    }

  printf ("Select an action:\n[1] Add rule\n[2] Remove rule\n");
  get_int (&action, 2, 1, 2, 1);
  switch (action)
    {
    case 1:
      get_user_input (&rule, (Table) table);
      add_rule (&rule);
      break;

    case 2:
      printf ("Enter the rule ID:\n");
      get_int (&id, 6, 0, G_MAXINT, 0);  // the max value of ID is 65535 (uint16)
      delete_rule ((guint16) id, (Table) table);
      break;

    default:
      printf ("Invalid action\n");
      return EXIT_FAILURE;
    }

  g_free (rule.name);
  return EXIT_SUCCESS;
}

// Get user's confirmation as 'y'; if confirmed, returns 1
gint confirm (void)
{
  // Reference [1]
  printf ("Do you want to continue? (y/N) ---> ");
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

void usage (void)
{
  printf ("Gawake (cli version): A Linux software to make your PC wake up on a scheduled time. "\
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
  printf ("\nUser interruption...\n");
  close_dbus_client ();
  exit (EXIT_FAILURE);
}

/* REFERENCES:
 * [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
 * [2] https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * [3] https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * [4] https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
 *
 * rtcwake manpage: https://www.man7.org/linux/man-pages/man8/rtcwake.8.html
 */
