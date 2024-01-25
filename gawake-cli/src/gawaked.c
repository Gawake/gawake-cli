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

#include "_include/gawaked.h"

// if pid == 0, it's the child process

int main (void)
{
  // FORK OFF THE PARENT PROCESS
  // PID: Process ID
  // SID: Session ID
  pid_t pid, sid;

  /* pid = fork (); */

  /* if (pid < 0) */
  /*   { */
  /*     // fork failed, no child process, end parent execution */
  /*     fprintf (stderr, "ERROR on fork ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  /* if (pid > 0) */
  /*   { */
  /*     // fork done, end parent process and continue to child process */
  /*     fprintf (stdout, "gawaked forked\n"); */
  /*     exit (EXIT_SUCCESS); */
  /*   } */

  /* // CREATE SID FOR CHILD */
  /* sid = setsid (); */
  /* if (sid < 0) */
  /*   { */
  /*     // Exit on fail */
  /*     fprintf (stderr, "ERROR on setsid ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  // CHANGE THE WORKING DIRECTORY
  /* if ((chdir ("/home/kelvin/")) < 0) */
  /*   { */
  /*     // Exit on fail */
  /*     fprintf (stderr, "ERROR on scheduler's chdir ()\n"); */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  // TODO drop privileges

  // CALL gawake-scheduler
  int fd[2];          // fd[0]: read; fd[1]: write
  pipe (fd);         // TODO error when return -1
  pid = fork ();

  if (pid < 0)
    {
      // Failed while forking gawake-scheduler, end gawaked (parent process)
      fprintf (stderr, "ERROR when forking to gawake-scheduler\n");
      exit (EXIT_FAILURE);
    }

  /* pipe_args_t *pipe_args; */
  /* pipe_args = malloc (pipe_args_s); */
  if (pid == 0)
    {
      // Child process: call gawake-scheduler
      RtcwakeArgs rtcwake_args;
      int scheduler_return;

      // Close the read file descriptor
      close (fd[0]);

      // https://stackoverflow.com/questions/28800809/how-to-create-a-child-process-with-a-different-name
      // Call scheduler function, passing pointer to arguments that must be filled
      scheduler_return = scheduler (&rtcwake_args);

      /* if (scheduler_return == EXIT_FAILURE) */
      /*   { */

      /*   } */
      /* pipe_args->hour = 5; pipe_args->minutes=M_30; pipe_args->mode=OFF; */
      /* write (fd[1], pipe_args, pipe_args_s); */

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

      /* read (fd[0], pipe_args, pipe_args_s); */
      /* printf ("hh %u mm %d mode %d\n", pipe_args->hour, pipe_args->minutes, pipe_args->mode); */

      close (fd[0]);
      /* free (pipe_args); */
    }


  // CLOSE FILE DESCRIPTORS
  /* close (STDIN_FILENO); */
  /* close (STDOUT_FILENO); */
  /* close (STDERR_FILENO); */

  return EXIT_SUCCESS;
}
