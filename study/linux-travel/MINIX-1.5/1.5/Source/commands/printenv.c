/* printenv - print the current environment	Author: Richard Todd */

main()
{
  extern char **environ;
  char **sptr;
  for (sptr = environ; *sptr; ++sptr) {
	prints("%s\n", *sptr);
  }
}
