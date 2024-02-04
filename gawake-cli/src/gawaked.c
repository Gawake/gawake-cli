/* gawaked.c
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

// TODO add sigterm

#include "_include/gawaked.h"
#include "debugger.h"

// if pid == 0, it's the child process

int main (void)
{
  // FORK OFF THE PARENT PROCESS
  // PID: Process ID
  // SID: Session ID
  pid_t pid, sid;

  // Init privileges utility (get gawake uid/gid)
  if (init_privileges ())
    exit (EXIT_FAILURE);

  // (Temporarily) drop privileges
  if (drop_privileges () == EXIT_FAILURE)
    exit (EXIT_FAILURE);

  /* pid = fork (); */

  /* if (pid < 0) */
  /*   { */
  /*     // fork failed, no child process, end parent execution */
  /*     DEBUG_PRINT_CONTEX; */
  /*     fprintf (stderr, "ERROR on fork ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  /* if (pid > 0) */
  /*   { */
  /*     // fork done, end parent process and continue to child process */
  /*     DEBUG_PRINT (("gawaked forked")); */
  /*     exit (EXIT_SUCCESS); */
  /*   } */

  /* // CREATE SID FOR CHILD */
  /* sid = setsid (); */
  /* if (sid < 0) */
  /*   { */
  /*     // Exit on fail */
  /*     DEBUG_PRINT_CONTEX; */
  /*     fprintf (stderr, "ERROR on setsid ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  // CHANGE THE WORKING DIRECTORY
  /* if ((chdir ("/home/kelvin/")) < 0) */
  /*   { */
  /*     // Exit on fail */
  /*     DEBUG_PRINT_CONTEX; */
  /*     fprintf (stderr, "ERROR on scheduler's chdir ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

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

      // Close write file descriptor
      close (fd[1]);

      // Wait for child process
      wait (NULL);

      if (read (fd[0], &rtcwake_args, RtcwakeArgs_s) == -1)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR when reading from pipe\n");
          close (fd[0]);
          exit (EXIT_FAILURE);
        }

      DEBUG_PRINT (("RtcwakeArgs fields read from pipe by gawaked process:\n"\
              "\tFound: %d\n\tShutdown: %d"\
              "\n\t(HH:MM) %02d:%02d (DD/MM/YYYY) %02d/%02d/%d"\
              "\n\tMode: %d",
              rtcwake_args.found, rtcwake_args.shutdown_fail,
              rtcwake_args.hour, rtcwake_args.minutes,
              rtcwake_args.day, rtcwake_args.month, rtcwake_args.year,
              rtcwake_args.mode));

      // Being paranoic for security, re-check the parameters;
      // It's needed anyway in case of the child process was abnormally ended
      if (validade_rtcwake_args (&rtcwake_args) == -1)
        {
          DEBUG_PRINT_CONTEX;
          fprintf (stderr, "ERROR: failed on rtcwake arguments validation\n");
          close (fd[0]);
          exit (EXIT_FAILURE);
        }

      raise_privileges ();
      // TODO run command
      drop_privileges_permanently ();

      close (fd[0]);
    }


  // CLOSE FILE DESCRIPTORS
  /* close (STDIN_FILENO); */
  /* close (STDOUT_FILENO); */
  /* close (STDERR_FILENO); */

  return EXIT_SUCCESS;
}
