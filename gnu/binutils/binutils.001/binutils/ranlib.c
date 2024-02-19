/* Dummy ranlib program for GNU.  All it does is
   `ar rs LIBRARY' for each library specified.  */

/* The makefile generates a -D switch to define AR_PROG
   as the location of the GNU AR program.  */

char *prog = AR_PROG;

main (argc, argv)
     int argc;
     char **argv;
{
  int i;
  for (i = 1; i < argc; i++)
    {
      int pid = fork ();
      if (pid == 0)
	{
	  execl (prog, prog, "rs", argv[i], 0);
	  perror (prog);
	  exit (1);
	}
      wait (0);
    }
  exit (0);
}
