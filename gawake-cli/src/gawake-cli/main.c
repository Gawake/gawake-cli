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

int main (int argc, char **argv)
{
  // Receiving arguments (reference [4])
  int cflag = 0, mflag = 0, sflag = 0;
  char *cvalue = NULL, *mvalue = NULL;
  int index;
  int c;
  opterr = 0;

  while ((c = getopt (argc, argv, "hsc:m:")) != -1)
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
          DEBUG_PRINT (("optarg: %s\n", optarg));
          if (optarg == NULL)
            return EXIT_FAILURE;
          else
            cvalue = optarg;

          if (strlen (cvalue) != 14)
            {
              fprintf (stderr, "Invalid time stamp. It must be on format \"YYYYMMDDhhmmss\".\n\n");
              return EXIT_FAILURE;
            }
          break;

        case 'm':
          mflag = 1;
          bool valid = false;
          if (optarg == NULL)
            return EXIT_FAILURE;
          else
            mvalue = optarg;

          for (int i = 0; i < 3; i++)
            {
              if (strcmp (mvalue, MODE[i]) == 0)
                {
                  valid = true; // If there is a valid mode, continue
                  break;
                }
            }
          // Exit on invalid mode
          if (!valid)
            {
              fprintf (stderr, "Invalid mode\n");
              return EXIT_FAILURE;
            }
          break;

        case '?':
          if (optopt == 'c' || optopt == 'm')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else if (isprint (optopt))
            fprintf (stderr, "Unknown option '-%c'.\n\n", optopt);
          else
            fprintf (stderr,
                     "Unknown option character '\\x%x'.\n",
                     optopt);
          return EXIT_FAILURE;

        default:
          abort ();
        }
    }

  // Case option 'c'
  if (cflag)
    {
      uint8_t month, day, hour, minutes, mode;
      uint16_t year;

      // Get timestamp
      if (sscanf (cvalue,
                  "%04"SCNu16 "%02"SCNu8 "%02"SCNu8 "%02"SCNu8 "%02"SCNu8,
                  &year, &month, &day, &hour, &minutes) != 5)
        {
          fprintf (stderr, "Invalid timestamp. It must be on format \"YYYYMMDDhhmmss\".\n");
          return EXIT_FAILURE;
        }

      // Get mode, if passed
      if (mflag)
        sscanf (mvalue, "%"SCNu8, &mode);
      else
        mode = OFF;

      if (connect_database ())
        return EXIT_FAILURE;

      custom_schedule (hour,
                       minutes,
                       day,
                       month,
                       year,
                       mode);

      close_database ();

      return EXIT_SUCCESS;
    }
  // Case option 's'
  else if (sflag)
    {
      if (mflag)
        {
          fprintf (stderr, "Mode is only supported with a custom timestamp (option '-c')\n");
          return EXIT_FAILURE;
        }

      if (connect_database ())
        return EXIT_FAILURE;

      schedule ();
      close_database ();

      return EXIT_SUCCESS;
    }
  // Case option 'm' only
  else if (mflag)
    {
      fprintf (stderr, "Mode is only supported with a custom timestamp (option '-c')\n");
      return EXIT_FAILURE;
    }

  // Exit if there are invalid arguments
  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);



  // If there's any arguments, continue to the menu
  printf ("Starting Gawake...\n");

  // If can't connect to the database, exit
  if (connect_database ())
    return EXIT_FAILURE;

  // Signal handler
  // Triggered on <Ctrl C>
  signal (SIGINT, exit_handler);

  menu ();

  // Close D-Bus connection
  close_database ();

  return EXIT_SUCCESS;
}

void menu (void)
{
  bool lock = true;
  char choice;

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
            {
              schedule ();
              lock = false;
            }
          break;

        case 'c':
          printf ("todo\n"); /* TODO config (); */
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
          lock = false;
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
  char choice[7];

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
  int c;
  while ((c = getchar()) != '\n' && c != EOF) { }
}

// Get the user input, on a valid rule format
void get_user_input (Rule *rule, Table table)
{
  const char DAYS_NAMES[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  bool invalid = true;     // This variable is a lock for invalid inputs
  char *pch;                  // To get the pointer of a new line char, and after, remove it

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
  int hour;
  get_int (&hour, 3, 0, 23, 1);
  rule->hour = (uint8_t) hour;

  // Minutes
  printf ("%-30s", "[Minutes] (from 00 to 59) ");
  invalid = true;
  int minutes;
  do
    {
      get_int (&minutes, 3, 0, 59, 1);

      if (minutes >= 0 && minutes <= 59)
        {
          invalid = false;
          rule->minutes = (uint8_t) minutes;
        }
      else
        invalid_value ();

    } while (invalid);

  // DAYS
  printf ("\nEnter the days the rule will be applied (1 for enabled, and 0 for disabled):\n");
  // For each day of the week, receive the user input
  int day;
  for (int i = 0; i < 7; i++) {
    day = 0;
    printf ("%-10s", DAYS_NAMES[i]);
    get_int (&day, 2, 0, 1, 1);
    rule->days[i] = (bool) day;
  }

  // ACTIVE
  rule->active = true; // when adding a rule, it's always active

  // MODE (only for turn off rules)
  if (table == T_OFF)
    {
      printf ("\nSelect a mode:\n");

      for (int i = 0; i < 3; i++)
        printf ("[%i]\t%s\n", i, MODE[i]);

      int mode;
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
void get_int (int *ptr, int digits, int min, int max, int repeat)
{
  char user_input[digits];    // Number of digits the user input must have
  int val;
  bool invalid = true;
  do
    {
      printf ("--> ");

      // If the fgets AND the sscanf are successful...
      if ((fgets (user_input, sizeof (user_input), stdin))
          && (sscanf (user_input, "%d", &val) == 1))
        {
          // ...check if the value is in the range and finishes the loop
          if (val >= min && val <= max)
            invalid = false;
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
              invalid = true;
            }
        }

      if (invalid && repeat)
        invalid_value (); // Print the invalid message only in case the repeat is required (and the input was invalid)
    } while (invalid && repeat); // Only repeat if the function was called with "repeat = 1"

  *ptr = val;
}

// Add or remove a rule
int add_remove_rule (void) {
  int table, action, id;
  Rule rule;

  rule.name = (char *) malloc (RULE_NAME_LENGTH);
  if (rule.name == NULL)
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("Couldn't allocate memory\n"));
      return EXIT_FAILURE;
    }

  if (print_rules (T_ON))
    return EXIT_FAILURE;

  if (print_rules (T_OFF))
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
      get_int (&id, 6, 0, INT_MAX, 0);  // the max value of ID is 65535 (uint16)
      delete_rule ((uint16_t) id, (Table) table);
      break;

    default:
      printf ("Invalid action\n");
      return EXIT_FAILURE;
    }

  free (rule.name);
  return EXIT_SUCCESS;
}

// Get user's confirmation as 'y'; if confirmed, returns 1
int confirm (void)
{
  // Reference [1]
  printf ("Do you want to continue? (y/N) ---> ");
  char choice = getchar ();
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
  printf ("Gawake (cli version): A Linux software to make your PC wake up on a scheduled time.\n"\
          "It makes the rtcwake command easier.\n"\
          "\nOptions:\n"\
          " -c\tSchedule with a custom timestamp (YYYYMMDDhhmmss); "\
          "if -m isn't set, uses \"off\" as the default mode\n"\
          " -h\tShow this help and exit\n"\
          " -m\tSet a mode; must be used together the '-c' option\n"\
          " -s\tDirectly run the schedule function, using the first upcoming turn on rule;\n"\
          "\tto use a custom timestamp use the '-c' option\n"\
          "\nExamples:\n"\
          " %-40sSchedule according to the next turn on rule\n"\
          " %-40sSchedule wake for 15 January 2025, at 09:45:00\n"\
          " %-40sSchedule wake for 28 December 2025, at 15:30:00; use mode disk\n\n",
          "gawake-cli -s", "gawake-cli -c 20250115094500", "gawake-cli -c 20251228153000 -m disk");
}

// Close the database and exit on <Ctrl C>
__attribute__((__noreturn__)) void exit_handler (int sig)
{
  printf ("\nUser interruption...\n");
  close_database ();
  exit (EXIT_FAILURE);
}

int print_rules (Table table)
{
  Rule *rules;
  uint16_t rowcount;
  if (query_rules (table, &rules, &rowcount))
    {
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, RED ("ERROR: Failed to query rules\n"));
      return EXIT_FAILURE;
    }


  // HEADER
  if (table == T_ON)
    {
      printf (GREEN (
              "[1] TURN ON RULES\n"\
              "┌─────┬─────────────────┬────────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬────────┐"\
              "\n│ %-4s│ %-16s│    Time    │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-7s│"),
              "ID", "Name", "Active");
    }
  else
    {
      printf (YELLOW (
              "[2] TURN OFF RULES\n"\
              "┌─────┬─────────────────┬────────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬────────┬─────────┐"\
              "\n│ %-4s│ %-16s│    Time    │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │ %-7s│ %-8s│"),
              "ID", "Name", "Active", "Mode");
    }

  // ROWS
  for (int counter = 0; counter < rowcount; counter++)
    {
      if (table == T_ON)
        {
          printf ("\n│ %03d │ %-16.15s│  %02d:%02d:00  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │   %-5d│",
                  rules[counter].id, rules[counter].name,
                  rules[counter].hour, rules[counter].minutes,
                  rules[counter].days[0], rules[counter].days[1], rules[counter].days[2], rules[counter].days[3],
                  rules[counter].days[4], rules[counter].days[5], rules[counter].days[6],
                  rules[counter].active);
        }
      else
        {
          printf ("\n│ %03d │ %-16.15s│  %02d:%02d:00  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │  %d  │   %-5d│ %-8s│",
                  rules[counter].id, rules[counter].name, rules[counter].hour, rules[counter].minutes,
                  rules[counter].days[0], rules[counter].days[1], rules[counter].days[2], rules[counter].days[3],
                  rules[counter].days[4], rules[counter].days[5], rules[counter].days[6],
                  rules[counter].active, MODE[rules[counter].mode]);
        }

      // Already free name, since it won't be used anymore
      free (rules[counter].name);
    }
  free (rules);

  // BOTTOM
  if (table == T_ON)
    {
      printf ("\n└─────┴─────────────────┴────────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴────────┘\n\n");
    }
  else
    {
      printf ("\n└─────┴─────────────────┴────────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴────────┴─────────┘\n");
    }

  return EXIT_SUCCESS;
}

/* REFERENCES:
 * [1] https://stackoverflow.com/questions/42318747/how-do-i-limit-my-user-input
 * [2] https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-getpwnam-access-user-database-by-user-name
 * [3] https://www.gnu.org/software/libc/manual/html_node/Setuid-Program-Example.html
 * [4] https://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Example-of-Getopt.html
 *
 * rtcwake manpage: https://www.man7.org/linux/man-pages/man8/rtcwake.8.html
 */
