/* machine.c, 27-03-1990, F. Meulenbroeks
 * usage: machine [machine name]
 *        chip [chip name]
 * where machine name or chip name are the same as the macros from 
 * <minix/config.h>
 * 
 * When executed without arguments this program returns either the machine name
 * or the chip name as found in <minix/config.h>
 * the chip name is returned if argv[0] ends on "chip". 
 * otherwise the machine name is returned
 *
 * when executed with an argument that argument is (case insensitive)
 * compared with the machine name, and depending on whether these match
 * or not 0 or 1 is returned as the exit code. In this case nothing is printed
 */

#include <minix/config.h>
#include <stdio.h>
#include <ctype.h>

main(argc, argv)
int argc;
char **argv;
{
  int i;
  char *ptr;
  
  if (argc > 1)
  {
    for (i = strlen(argv[1]) - 1; i >= 0; i--) toupper(argv[1][i]);
  }

  /* let ptr point to the last 4 characters of argv[0] */
  ptr = argv[0] + strlen(argv[0]) - 4;
  if (strncmp(ptr, "chip", 4) == 0)
  {
#if (CHIP == INTEL)
    if (argc == 1) {
      printf("INTEL\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "INTEL") == 0) exit(0);
    else exit(1);
#endif

#if (CHIP == M68000)
    if (argc == 1) {
      printf("M68000\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "M68000") == 0) exit(0);
    else exit(1);
#endif

#if (CHIP == SPARC)
    if (argc == 1) {
      printf("SPARC\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "SPARC") == 0) exit(0);
    else exit(1);
#endif

#if (CHIP != INTEL) & (CHIP != M68000) & (CHIP != SPARC)
    if (argc == 1) printf("unknown\n");
    exit(1);
#endif
  }
  else {
#if (MACHINE == IBM_PC)
    if (argc == 1) {
      printf("IBM_PC\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "IBM_PC") == 0) exit(0);
    else exit(1);
#endif

#if (MACHINE == SUN_4)
    if (argc == 1) {
      printf("SUN_4\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "SUN_4") == 0) exit(0);
    else exit(1);
#endif

#if (MACHINE == ATARI)
    if (argc == 1) {
      printf("ATARI\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "ATARI") == 0) exit(0);
    else exit(1);
#endif

#if (MACHINE == AMIGA)
    if (argc == 1) {
      printf("AMIGA\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "AMIGA") == 0) exit(0);
    else exit(1);
#endif

#if (MACHINE == MACINTOSH)
    if (argc == 1) {
      printf("MACINTOSH\n");
      exit(0);
    }
    else
    if (strcmp(argv[1], "MACINTOSH") == 0) exit(0);
    else exit(1);
#endif

#if (MACHINE != IBM_PC) & \
    (MACHINE != SUN_4) & \
    (MACHINE != ATARI) & (MACHINE != AMIGA) & (MACHINE != MACINTOSH)
    if (argc == 1) printf("unknown\n");
    exit(1);
#endif
  }
}
