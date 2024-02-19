/*			E x e r c i s e
 *
 * This program exercises the stdio routines. It does not validate the
 * routines but provides a convenient way to quickly check the functionality
 * of the code.
 */

#ifdef _BSD
# include <strings.h>
#else
# include <stdlib.h>
# include <string.h>
#endif
#include <stdio.h>

#define UCHAR(x)	((int) ((x) & 0xff))

#define DEC -123
#define INT 255
#define UNS (~0)
#define TESTFILE	"test.dat"
#define LARGEBUFS	16
#ifdef		MSDOS
# define	TTY	"con"
#else
# define	TTY	"/dev/tty"
#endif

extern void *malloc();			/* memory allocator */
extern unsigned sleep();		/* sleep routine */
extern char *strcpy();			/* string copy */
extern char *strcat();			/* string concatenation */
extern int strcmp();			/* string compare */
extern void exit();			/* exit */

FILE *fp;				/* per test file pointer */

/*
 * Line Buffered Write Test
 *
 * Write to a terminal. This tests that the output buffer is
 * flushed on receipt of a \n.
 */

void lbw_test()

{
  int i;

  puts("\nLine buffered write test");
  if ((fp = fopen(TTY, "w")) != NULL) {
    puts("<pause>ABCDEFGH");
    puts("<pause>ABCD<pause>EFGH");
    for (i = 0; i < 8; i++)
      putc('A'+i, fp), sleep(1);
    putc('\n', fp);
    for (i = 0; i < 8; i++) {
      putc('A'+i, fp);
      if (i == 3)
	fflush(fp);
      sleep(1);
    }
    fclose(fp);
    puts("");
  }
}

/*
 * Unbuffered Write Test
 *
 * Test that characters are written directly to the output device
 * when the stream is unbuffered.
 */

void ubw_test()

{
  int i;

  puts("\nUnbuffered write test");
  if ((fp = fopen(TTY, "w")) != NULL) {
    setbuf(fp, (char *) 0);
    puts("A<pause>B<pause>C<pause>D<pause>E<pause>F<pause>G<pause>H<pause>");
    puts("A<pause>B<pause>C<pause>D<pause>E<pause>F<pause>G<pause>H<pause>");
    for (i = 0; i < 8; i++)
      putc('A'+i, fp), sleep(1);
    putc('\n', fp);
    for (i = 0; i < 8; i++)
      putc('A'+i, fp), sleep(1);
    fclose(fp);
    puts("");
  }
}

/*
 * Buffered Write Test
 *
 * Test that the data is written to the terminal on a per buffer
 * basis.
 */

void bw_test()

{
  int i;

  puts("\nFully buffered write test");
  if ((fp = fopen(TTY, "w")) != NULL) {
    setvbuf(fp, (char *) 0, _IOFBF, 4);
    puts("<pause>ABCD<pause>EFGH<pause>");
    puts("ABC<pause>DEFG<pause>H");
    for (i = 0; i < 8; i++)
      putc('A'+i, fp), sleep(1);
    putc('\n', fp);
    for (i = 0; i < 8; i++)
      putc('A'+i, fp), sleep(1);
    fclose(fp);
    puts("");
  }
}

/* Formatted Output Test
 *
 * This exercises the output formatting code.
 */

void fp_test()

{
  int i, j, k, l;
  char buf[7];
  char *prefix = buf;
  char tp[20];

  puts("\nFormatted output test");
  printf("prefix  6d      6o      6x      6X      6u\n");
  strcpy(prefix, "%");
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      for (k = 0; k < 2; k++) {
	for (l = 0; l < 2; l++) {
	  strcpy(prefix, "%");
	  if (i == 0) strcat(prefix, "-");
	  if (j == 0) strcat(prefix, "+");
	  if (k == 0) strcat(prefix, "#");
	  if (l == 0) strcat(prefix, "0");
	  printf("%5s |", prefix);
	  strcpy(tp, prefix);
	  strcat(tp, "6d |");
	  printf(tp, DEC);
	  strcpy(tp, prefix);
	  strcat(tp, "6o |");
	  printf(tp, INT);
	  strcpy(tp, prefix);
	  strcat(tp, "6x |");
	  printf(tp, INT);
	  strcpy(tp, prefix);
	  strcat(tp, "6X |");
	  printf(tp, INT);
	  strcpy(tp, prefix);
	  strcat(tp, "6u |");
	  printf(tp, UNS);
	  printf("\n");
	}
      }
    }
  }
}

/*
 * String Output Test
 *
 * Test the string printf code.
 */

void sw_test()

{
  int i;
  char buf[80];

  puts("\nTest sprintf functionality");
  puts("13 bytes in 'Testing 1 2 3'");
  i = sprintf(buf, "Testing %d %d %d", 1, 2, 3);
  printf("%d bytes in '%s'\n", i, buf);
}

/*
 * String Input Test
 *
 * Test the string scanf code.
 */

void sr_test()

{
  int i, j;
  char buf[80];

  puts("\nTest sscanf functionality");
  puts("2 items yielding 25 and 'thompson'");
  i = sscanf("25 thompson", "%d%s", &j, buf);
  printf("%d items yielding %d and '%s'\n", i, j, buf);
}

/*
 * File Write and Read Test
 *
 * Test that a file can be written to and read from.
 */

void frw_test()

{
  int i, j, k;
  char buf[80];

  puts("\nFile write and read check");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    puts("3 items yielding 56, 789 and '56'");
    puts("1 item yielding 'a72'");
    fprintf(fp, "56789 0123 56a72");
    if (freopen(TESTFILE, "r", fp) != fp)
      puts("Cannot open file for reading");
    else {
      i = fscanf(fp, "%2d%d%*d %[0-9]", &j, &k, buf);
      printf("%d items yielding %d, %d and '%s'\n", i, j, k, buf);
      i = fscanf(fp, "%s", buf);
      printf("%d item yielding '%s'\n", i, buf);
      fclose(fp);
    }
  }
}

/*
 * File Seek Test
 *
 * Test that seek operations within files work.
 */

void fs_test()

{
  int i, j;

  puts("\nFile seek test");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    for (i = 0; i < 256; i++)
      putc(i, fp);
    if (freopen(TESTFILE, "r", fp) != fp)
      puts("Cannot open file for reading");
    else {
      for (i = 1; i <= 255; i++) {
        printf("\r%3d ", i);
	fflush(stdout);
        fseek(fp, (long) -i, SEEK_END);
	if ((j = getc(fp)) != 256-i) {
	  printf("SEEK_END failed %d\n", j);
	  break;
	}
	if (fseek(fp, (long) i, SEEK_SET)) {
	  puts("Cannot SEEK_SET");
	  break;
	}
	if ((j = getc(fp)) != i) {
	  printf("SEEK_SET failed %d\n", j);
	  break;
	}
	if (fseek(fp, (long) i, SEEK_SET)) {
	  puts("Cannot SEEK_SET");
	  break;
	}
	if (fseek(fp, (long) (i >= 128 ? -128 : 128), SEEK_CUR)) {
	  puts("Cannot SEEK_CUR");
	  break;
	}
	if ((j = getc(fp)) != (i >= 128 ? i-128 : i+128)) {
	  printf("SEEK_CUR failed %d\n", j);
	  break;
	}
      }
      if (i > 255)
	puts("ok");
      fclose(fp);
    }
  }
}

/*
 * Test gets()
 *
 * Checks that gets() works.
 */

void gets_test()

{
  char buf[80];

  puts("\nGets functionality");
  puts("... Type a line and have it echoed ...");
  gets(buf);
  puts(buf);
}

/*
 * Fputs Test
 *
 * Check that fputs() works into unbuffered streams.
 */

void fputs_test()

{
  puts("\nUnbuffered fputs test");
  if ((fp = fopen(TTY, "w")) != NULL) {
    setbuf(fp, (char *) 0);
    puts("ABCDEFGH<pause>");
    puts("ABCDEFGH<pause>");
    fputs("ABCDEFGH", fp), sleep(1);
    fputs("\nABCDEFGH", fp), sleep(1);
    fclose(fp);
    puts("");
  }
}

/*
 * Fprintf Test
 *
 * Check that fprintf() works into unbuffered streams.
 */

void fprint_test()

{
  puts("\nUnbuffered fprintf test");
  if ((fp = fopen(TTY, "w")) != NULL) {
    setbuf(fp, (char *) 0);
    puts("ABCDEFGH<pause>");
    puts("ABCDEFGH<pause>");
    fprintf(fp, "ABCDEFGH"), sleep(1);
    fprintf(fp, "\nABCDEFGH"), sleep(1);
    fclose(fp);
    puts("");
  }
}

/*
 * Fgets Test
 *
 * Check that fgets() works.
 */

void fgets_test()

{
  char buf[80];

  puts("\nFgets functionality");
  puts("a");
  puts("<pause>ab");
  puts("<pause>abc");
  puts("<pause>abcd");
  puts("<pause>abcde");
  puts("<pause>abcdef");
  puts("<pause>abcdefg<pause>");
  puts("<pause>abcdefg<pause>h");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    fputs("a\n", fp);
    fputs("ab\n", fp);
    fputs("abc\n", fp);
    fputs("abcd\n", fp);
    fputs("abcde\n", fp);
    fputs("abcdef\n", fp);
    fputs("abcdefg\n", fp);
    fputs("abcdefgh\n", fp);
    fclose(fp);
    if ((fp = fopen(TESTFILE, "r")) != NULL) {
      while (fgets(buf, 8, fp) != NULL) {
	fputs(buf, stdout);
	fflush(stdout);
	sleep(1);
      }
      fclose(fp);
    }
  }
}

/*
 * Word Read and Write Test
 *
 * Check that putw and getw work.
 */

void word_test()

{
  int i, j;

  puts("\nPutw and Readw Test");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    for (i = 0; i < 256; i++)
      putw(i, fp);
    putc(0, fp);
    fclose(fp);
    if ((fp = fopen(TESTFILE, "r")) != NULL) {
      for (i = 0; i < 256; i++) {
	printf("\r%3d", i);
	fflush(stdout);
	if ((j = getw(fp)) != i) {
	  printf(" failed %d", j);
	  break;
	}
      }
      if (i == 256 /* && getw(fp) == EOF && feof(fp)*/)
	fputs(" ok", stdout);
      puts("");
      fclose(fp);
    }
  }
}

/*
 * Append Test
 *
 * Check that appends go to the end of the file.
 */

void a_test()

{
  int ch;

  puts("\nAppend Test");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    putc('a', fp);
    if ((fp = freopen(TESTFILE, "a", fp)) == NULL) {
      puts("Cannot freopen file");
      return;
    }
    if (fseek(fp, 0L, 0) == EOF) {
      puts("Cannot fseek to start");
      return;
    }
    putc('@', fp);
    if ((fp = freopen(TESTFILE, "r", fp)) == NULL) {
      puts("Cannot freopen file");
      return;
    }
    if ((ch = getc(fp)) != 'a')
      printf("Failed a - %c\n", ch);
    else if ((ch = getc(fp)) != '@')
      printf("Failed @ - %c\n", ch);
    else if ((ch = getc(fp)) != EOF)
      printf("Failed EOF - %d\n", ch);
    else
      puts("Ok");
    fclose(fp);
    return;
  }
}

/*
 * Write and Read Update Test
 *
 * Write a file in update mode, then try to read it.
 */

void uwr_test()

{
  int i, j;

  puts("\nWrite and Read Update Test");
  if ((fp = fopen(TESTFILE, "w+")) != NULL) {
    for (i = 0; i < (3*BUFSIZ)/2; i++)
      putc(i, fp);
    rewind(fp);
    for (i = 0; i < (3*BUFSIZ)/2; i++) {
      printf("\r%4d", i);
      fflush(stdout);
      j = getc(fp);
      if (j != UCHAR(i)) {
	printf(" failed %d\n", j);
	break;
      }
    }
    if (i == (3*BUFSIZ)/2)
      puts(" ok");
    if (getc(fp) != EOF)
      puts(" failed to find eof");
    else {
      for (i = 0; i < BUFSIZ/2; i++)
	putc(i, fp);
      fseek(fp, (long) (3*BUFSIZ)/2, SEEK_SET);
      for (i = 0; i < BUFSIZ/2; i++) {
	printf("\r%4d", i);
	fflush(stdout);
	j = getc(fp);
	if (j != UCHAR(i)) {
	  printf(" failed %d\n", j);
	  break;
	}
      }
      if (i == BUFSIZ/2)
	puts(" ok");
    }
    fclose(fp);
  }
}

/*
 * Write, Append and Read Update Test
 *
 * Write a file in update mode, close it, append to it and read it.
 */

void uawr_test()

{
  int i, j;

  puts("\nWrite, Append and Read Update Test");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    for (i = 0; i < (3*BUFSIZ)/2; i++)
      putc(i, fp);
    fclose(fp);
    if ((fp = fopen(TESTFILE, "a+")) != NULL) {
      for (i = 0; i < BUFSIZ/2; i++)
	putc(i, fp);
      fseek(fp, (long) (3*BUFSIZ)/2, SEEK_SET);
      for (i = 0; i < BUFSIZ/2; i++) {
	printf("\r%4d", i);
	fflush(stdout);
	j = getc(fp);
	if (j != UCHAR(i)) {
	  printf(" failed %d\n", j);
	  break;
	}
      }
      if (i == BUFSIZ/2)
	puts(" ok");
      rewind(fp);
      for (i = 0; i < (3*BUFSIZ)/2; i++) {
	printf("\r%4d", i);
	fflush(stdout);
	j = getc(fp);
	if (j != UCHAR(i)) {
	  printf(" failed at %d\n", j);
	  break;
	}
      }
      if (i == (3*BUFSIZ)/2)
	puts(" ok");
    }
    fclose(fp);
  }
}

/*
 * Write, Read, Write and Read Update Test
 *
 * Write a file in update mode, read it, write it and read it.
 */

void uwrwr_test()

{
  int i, j;

  puts("\nWrite, Read, Write and Read Update Test");
  if ((fp = fopen(TESTFILE, "w")) != NULL) {
    for (i = 0; i < (3*BUFSIZ)/2; i++)
      putc(i, fp);
    fclose(fp);
    if ((fp = fopen(TESTFILE, "r+")) != NULL) {
      for (i = 0; i < (3*BUFSIZ)/2; i++) {
	printf("\r%4d", i);
	fflush(stdout);
	j = getc(fp);
	if (j != UCHAR(i)) {
	  printf(" failed %d\n", j);
	  break;
	}
      }
      if (i == (3*BUFSIZ)/2)
	puts(" ok");
      if (getc(fp) != EOF)
	puts(" failed to find eof");
      else {
	for (i = 0; i < BUFSIZ/2; i++)
	  putc(i, fp);
	rewind(fp);
	for (i = 0; i < (3*BUFSIZ)/2; i++)
	  putc((3*BUFSIZ)/2-i, fp);
	fseek(fp, (long) (3*BUFSIZ)/2, SEEK_SET);
	for (i = 0; i < BUFSIZ/2; i++) {
	  printf("\r%4d", i);
	  fflush(stdout);
	  j = getc(fp);
	  if (j != UCHAR(i)) {
	    printf(" failed %d\n", j);
	    break;
	  }
	}
	if (i == BUFSIZ/2)
	  puts(" ok");
	rewind(fp);
	for (i = 0; i < (3*BUFSIZ)/2; i++) {
	  printf("\r%4d", i);
	  fflush(stdout);
	  j = getc(fp);
	  if (j != UCHAR((3*BUFSIZ)/2-i)) {
	    printf(" failed %d\n", j);
	    break;
	  }
	}
	if (i == (3*BUFSIZ)/2)
	  puts(" ok");
      }
      fclose(fp);
    }
  }
}

/*
 * Fwrite Test
 *
 * Test fwrite with small loads and large loads.
 */

void fwrite_test()

{
  unsigned int i, j;
  char buf[1023];
  char bbuf[3071];
  double sqrt();
  void free();
  char *p;

  puts("\nFwrite Test");
  if ((fp = fopen(TESTFILE, "w+")) != NULL) {

    for (i = 0; i < sizeof(buf); i++)
      buf[i] = i;
    for (i = 0; i < sizeof(bbuf); i++)
      bbuf[i] = 19-i;

    for (i = 0; i < 256; i++) {
      printf("\r%4d", i);
      fflush(stdout);
      if (fwrite(buf, 1, sizeof(buf), fp) != sizeof(buf)) {
	puts(" failed\n");
	return;
      }
      putc(0, fp);
    }
    puts(" small write ok");
    rewind(fp);
    for (i = 0; i < 256; i++) {
      printf("\r%4d", i);
      fflush(stdout);
      for (j = 0; j < sizeof(buf); j++) {
	if (getc(fp) != UCHAR(j)) {
	  puts(" failed\n");
	  return;
	}
      }
      if (getc(fp) != 0) {
	puts(" failed\n");
	return;
      }
    }
    puts(" verified ok");

    rewind(fp);
    for (i = 0; i < 256; i++) {
      printf("\r%4d", i);
      fflush(stdout);
      if (fwrite(bbuf, 1, sizeof(bbuf), fp) != sizeof(bbuf)) {
	puts(" failed\n");
	return;
      }
      putc(0, fp);
    }
    puts(" large write ok");
    rewind(fp);
    for (i = 0; i < 256; i++) {
      printf("\r%4d", i);
      fflush(stdout);
      for (j = 0; j < sizeof(bbuf); j++) {
	if (getc(fp) != UCHAR(19-j)) {
	  puts(" failed\n");
	  return;
	}
      }
      if (getc(fp) != 0) {
	puts(" failed\n");
	return;
      }
    }
    puts(" verified ok");

    rewind(fp);
    if ((p = (char *) malloc(48*1024)) == 0) {
      puts("No memory for large write test");
      return;
    }
    for (j = 13, i = 48*1024; --i; j++)
      p[i] = j;
    fwrite(p, 48*1024, 1, fp);
    rewind(fp);
    for (i = 48*1024; --i; )
      p[i] = 0;
    fread(p, 48*1024, 1, fp);
    for (j = 13, i = 48*1024; --i; j++) {
      if (i % 1024 == 0) {
	printf("\r%5u", i);
	fflush(stdout);
      }
      if (p[i] != (char) j) {
	printf("\r%5u failed %d instead of %d\n", i, p[i], UCHAR(j));
	free(p);
	return;
      }
    }
    printf("\r%5u ok\n", i);
    free(p);
  }
}

/*
 * Test the exit code
 *
 * Load an exit handler and check buffer flushing.
 */

static void handler()

{
  fputs("Exit handler called ok\n", fp);
  fflush(fp);
  fputs("Buffer flush ok\n", fp);
  sleep(2);
}

void exit_test()

{
  int atexit();

  puts("\nExit Test");
  if ((fp = fopen(TTY, "w")) == NULL) {
    puts("Cannot open tty for exit test");
    return;
  }
  setvbuf(fp, (char *) 0, _IOFBF, BUFSIZ);
  if (atexit(handler) != 0)
    puts("Exit handler not lodged");
}

/* Temporary File Test
 *
 * Check the names produced by tmpnam.
 */

void tmp_test()

{
  int i;
  char buf[20];
  char *tf;

  puts("\nTemporary File Names");
  for (i = 10; i--; ) {
    tf = tmpnam((char *) 0);
    fputs(tf, stdout);
    if (strlen(tf) == L_tmpnam-1)
      puts(" ok");
    else
      puts(" failed");
  }
  if ((fp = tmpfile()) == 0) {
    puts("Cannot make temporary file");
    return;
  }
  printf("Temporary file");
  fputs("123456", fp);
  rewind(fp);
  fgets(buf, 20, fp);
  if (strcmp(buf, "123456") != 0)
    puts(" failed");
  else
    puts(" ok");
}

/* Id test
 */

void id_test()

{
  fputs("User id  : ", stdout);
  puts(cuserid((char *) 0));
  fputs("Terminal : ", stdout);
  puts(ctermid((char *) 0));
}

int main()

{
  id_test();
  lbw_test();
  ubw_test();
  bw_test();
  fp_test();
  sw_test();
  sr_test();
  frw_test();
  fs_test();
  fputs_test();
  fprint_test();
  gets_test();
  fgets_test();
  word_test();
  fwrite_test();
  a_test();
  uwr_test();
  uawr_test();
  uwrwr_test();
  tmp_test();
  exit_test();
  remove(TESTFILE);
  return 0;
}
