/* main.c
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

// Main file for gawaked

#include "main.h"
#include "../utils/debugger.h"

// if pid == 0, it's the child process

// PID: Process ID
static pid_t pid;

int main (void)
{
  // Init privileges utility (get gawake uid/gid)
  if (init_privileges ())
    exit (EXIT_FAILURE);

  // (Temporarily) drop privileges
  if (drop_privileges () == EXIT_FAILURE)
    exit (EXIT_FAILURE);

  // Signals for terminating the service by Systemd
  signal (SIGTERM, exit_handler);

  // CLOSE FILE DESCRIPTORS
  close (STDIN_FILENO);
#if PREPROCESSOR_DEBUG == 0
  close (STDOUT_FILENO);
#endif

  // CALL scheduler
  int fd[2];          // fd[0]: read; fd[1]: write
  if (pipe (fd) == -1)
    {
      // Error when trying to pipe, child process not create, exit parent process
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "Error on pipe\n");
      exit (EXIT_FAILURE);
    }
  pid = fork ();

  if (pid < 0)
    {
      // Failed while forking scheduler, end gawaked (parent process)
      DEBUG_PRINT_CONTEX;
      fprintf (stderr, "ERROR when forking to gawake-scheduler\n");
      exit (EXIT_FAILURE);
    }

  RtcwakeArgs rtcwake_args;
  if (pid == 0)
    {
      // Child process: executed as gawake user, with no possibility to setuid to root

      // If drop privileges permanently or the check user fails, end execution
      if (drop_privileges_permanently () || check_user ())
        {
          exit (EXIT_FAILURE);
        }

      // Set process name
      if (prctl (PR_SET_NAME, "scheduler") < 0)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "Failed to set child process name");
          // No need to exit
        }

      // Call gawake-scheduler:
      int scheduler_return;

      // Close the read file descriptor
      close (fd[0]);

      // Call scheduler function, passing pointer to arguments that must be filled
      scheduler_return = scheduler (&rtcwake_args);

      DEBUG_PRINT (("RtcwakeArgs fields returned by scheduler ():\n"\
                    "\tFound: %d\n\tShutdown: %d"\
                    "\n\t(HH:MM) %02d:%02d (DD/MM/YYYY) %02d/%02d/%d"\
                    "\n\tMode: %d",
                    rtcwake_args.found, rtcwake_args.shutdown_fail,
                    rtcwake_args.hour, rtcwake_args.minutes,
                    rtcwake_args.day, rtcwake_args.month, rtcwake_args.year,
                    rtcwake_args.mode));

      // If scheduler failed, end child process
      if (scheduler_return == EXIT_FAILURE)
        {
          close (fd[1]);
          exit (EXIT_SUCCESS);
        }
      else
        {
          // If write to pipe fails, end child process
          if (write (fd[1], &rtcwake_args, RtcwakeArgs_s) == -1)
            {
              DEBUG_PRINT_CONTEX;
              fprintf (stderr, "ERROR when writing to pipe\n");
              close (fd[1]);
              exit (EXIT_FAILURE);
            }
        }

      close (fd[1]);

      exit (EXIT_SUCCESS);
    }
  else if (pid > 0)
    {
      // Parent process
      int exit_status;

      // Close write file descriptor
      close (fd[1]);

      // Wait for child process
      wait (&exit_status);

      // Check the return of the child process
      if (WIFEXITED (exit_status))
        {
          // if the child exited normally, get its return code
          int return_value = WEXITSTATUS (exit_status);
          DEBUG_PRINT (("scheduler child return value: %d", return_value));

          if (return_value != EXIT_SUCCESS)
            {
              DEBUG_PRINT_CONTEX;
              fprintf (stderr, "ERROR: scheduler child process exited with code: %d; "\
                       "terminating parent process\n", return_value);
              close (fd[0]);
              exit (EXIT_FAILURE);
            }
        }
      else
        {
          // child didn't exited normally
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR: scheduler child process exited unsuccessfully; "\
                   "terminating parent process\n");
          close (fd[0]);
          exit (EXIT_FAILURE);
        }

      // Read values returned from child
      if (read (fd[0], &rtcwake_args, RtcwakeArgs_s) == -1)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR when reading from pipe\n");
          close (fd[0]);
          exit (EXIT_FAILURE);
        }
      // close pipe, as it won't be used anymore
      close (fd[0]);

      DEBUG_PRINT (("RtcwakeArgs fields read from pipe by gawaked process:\n"\
              "\tFound: %d\n\tShutdown: %d"\
              "\n\t(HH:MM) %02d:%02d (DD/MM/YYYY) %02d/%02d/%d"\
              "\n\tMode: %d",
              rtcwake_args.found, rtcwake_args.run_shutdown,
              rtcwake_args.hour, rtcwake_args.minutes,
              rtcwake_args.day, rtcwake_args.month, rtcwake_args.year,
              rtcwake_args.mode));

      // Being paranoic for security, re-check the parameters;
      if (validade_rtcwake_args (&rtcwake_args) == -1)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR: failed on rtcwake arguments validation\n");
          exit (EXIT_FAILURE);
        }

      // Run shutdown if it's the case
      if (rtcwake_args.run_shutdown)
        {
          DEBUG_PRINT (("Shutdown"));
          system (SHUTDOWN);
          exit (EXIT_SUCCESS);
        }

      // Prepare command
      char *command;
      command = (char *) malloc (COMMAND_LENGTH * sizeof (char));
      if (command == NULL)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR: couldn't allocate memory\n");
          exit (EXIT_FAILURE);
        }
      snprintf (command,
                COMMAND_LENGTH,
                COMMAND_BEGINNING COMMAND_ARGUMENTS
                COMMAND_TIMESTAMP "%d%02d%02d%02d%02d00"
                COMMAND_MODE "%s",
                rtcwake_args.year, rtcwake_args.month, rtcwake_args.day, rtcwake_args.hour, rtcwake_args.minutes,
#if MODE_ALWAYS_ON
                // Set mode to "on" if this is a developing/debug version
                "on");
#else
                MODE[rtcwake_args.mode]);
#endif

      DEBUG_PRINT (("Command: %s\nLength: %ld", command, COMMAND_LENGTH));

      raise_privileges ();
      system (command);
      drop_privileges_permanently ();

      free (command);
    }

  return EXIT_SUCCESS;
}

// TODO remove this function (?)
static void exit_handler (int sig)
{
  // Terminate child process first (notice that it just sends a signal and doesn't wait)
  /* kill (pid, SIGTERM); */

  DEBUG_PRINT (("gawaked process terminated by SIGTERM"));

  exit (EXIT_SUCCESS);
}

