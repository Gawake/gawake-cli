#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#include "scheduler.h"
#include "gawake-service.h"

// if pid == 0, it's the child process

int _scheduler (void)
{
  fprintf (stdout, "Starting Gawake's scheduler...\n");

  // FORK OFF THE PARENT PROCESS
  // PID: Process ID
  // SID: Session ID
  pid_t pid, sid;


  pid = fork ();

  if (pid < 0)
    {
      // fork failed, no child process, end parent execution
      fprintf (stderr, "ERROR on fork ()\n");
      exit (EXIT_FAILURE);
    }

  if (pid > 0)
    {
      // fork done, end parent process and continue to child process
      fprintf (stdout, "Gawake's scheduler forked.\n");
      exit (EXIT_SUCCESS);
    }

  // CREATE SID FOR CHILD
  sid = setsid ();
  if (sid < 0)
    {
      // Exit on fail
      fprintf (stderr, "ERROR on setsid ()\n");
      exit (EXIT_FAILURE);
    }

  // CHANGE THE WORKING DIRECTORY
  // TODO should it be / or something else?
  if ((chdir ("/home/kelvin/")) < 0)
    {
      // Exit on fail
      fprintf (stderr, "ERROR on scheduler's chdir ()\n");
      exit (EXIT_FAILURE);
    }

  // CALL gawake-service
  /* int fd[2]; */
  /* char buffer[2048]; */
  pid = fork ();
  if (pid < 0)
    {
      // Exit on fail
      fprintf (stderr, "ERROR when forking to gawake-service\n");
      exit (EXIT_FAILURE);
    }

  if (pid == 0)
    {
      fprintf (stdout, "Starting gawake-service\n");
      gawake_service ();
    }

  if (pid != 0)
    {
      wait (NULL);
      printf ("returned \n");
    }


  // CLOSE FILE DESCRIPTORS
  /* close (STDIN_FILENO); */
  /* close (STDOUT_FILENO); */
  /* close (STDERR_FILENO); */

  return EXIT_SUCCESS;
}
