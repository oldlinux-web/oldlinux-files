/* recover - un-remove a file		Author: Andy Tanenbaum */

/* Unlike most UNIX systems, MINIX provides a way to recover a file that
 * has been accidently rm'ed.  The recovery is done using Terrence Holm's
 * 'de' (disk editor) program, plus some patches to FS that keep the i-node
 * number in the directory, even after the file has been removed.  A file
 * cannot be recovered after its directory slot has been reused.
 *
 * Usage:
 *	recovery file ...
 *
 * Note: the file names must be fully explicit; wild cards are not allowed.
 * It is not possible, for example, to say recover *.c.  All the files must
 * be named in full.  Since the last two bytes of the directory entry are
 * used for the i-node number, only the first 12 characters of the file name
 * count.  Full 14 character file names can be specified, however, only the
 * last two characters, in fact, play no role in locating the file.
 */

main(argc, argv)
int argc;
char *argv[];
{
  int i;
  char buf[1024];

  if (argc == 1) usage();

  for (i = 1; i < argc; i++) {
	strcpy(buf, "de -r ");
	strcat(buf, argv[i]);
	system(buf);
  }
}


usage()
{
  std_err("Usage: recover file ...\n");
  exit(1);
}
