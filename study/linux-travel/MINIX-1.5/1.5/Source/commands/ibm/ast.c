/* ast - add symbol table.		Author: Dick van Veen */

#include <sys/types.h>
#include <fcntl.h>
#include <a.out.h>
#include <stdio.h>

/* Since the a.out file in MINIX does not contain any symbol table,
 * we use the symbol table produced with the -s option of asld.
 *
 * Read symbol table in memory, remove compiler generated labels,
 * sort the labels and add it to the a.out file.
 *
 * When finally there comes a real as and ld, we may also get
 * a symbol table in the a.out file, and we can forget this program.
 *
 */

/* Ast [flags] [file] [symbolfile]
 *
 * flags:
 *	-x	do not preserve local symbols
 *	-X	preserve local symbols except for those whose name begin
 *		with 'I', these are compiler generated.
 *
 *	-	when no symbol file is present, symbol.out is assumed.
 *	-	when no file is present, a.out is assumed.
 *	-	when one file name is present it must be the executable file
 *	-	just one flag may be pressent.
 *
 */

#define A_OUT		"a.out"
#define SYMBOL_FILE	"symbol.out"	/* contains symbol table */
#define LINE_LENGTH	24

#define WORTH_LESS	1	/* lines contain no symbol */
#define LAST_LINE	2	/* end of file reached */

struct exec header;		/* header info of a.out file */

int x_flag;			/* flags to ast */
int X_flag;
int o_flag;

char *s_file, *o_file;		/* names of files used by ast */
FILE *s_fd, *o_fd;		/* file descriptors of those files */
int nr_symbols;			/* number of symbols added */
char buffer[LINE_LENGTH];	/* contains line of symbol file */

char io_buf[BUFSIZ];		/* for buffered output on stderr */
unsigned int get_value();	/* forward definition */

main(argc, argv)
int argc;
char **argv;
{
  extern FILE *fopen();

  argv++;
  if (*argv != NULL && **argv == '-') {
	*argv += 1;
	if (**argv == 'x')
		x_flag = 1;
	else if (**argv == 'X')
		X_flag = 1;
	else {
		fprintf(stderr, "illegal flag: -%c\n", **argv);
		exit(-1);
	}
	argv++;
  }
  if (*argv != NULL) {
	o_file = *argv;
	argv++;
  }
  if (*argv != NULL) {
	s_file = *argv;
	argv++;
  }
  if (*argv != NULL) {
	fprintf(stderr, "Usage: ast [-{x,X}] [file] [symbolfile]\n");
	exit(-1);
  }
  if (o_file == NULL) o_file = A_OUT;
  o_fd = fopen(o_file, "a");
  if (o_fd == NULL) {
	fprintf(stderr, "can't open %s\n", o_file);
	exit(-1);
  }
  if (s_file == NULL) s_file = SYMBOL_FILE;
  s_fd = fopen(s_file, "r");
  if (s_fd == NULL) {
	fprintf(stderr, "can't open %s\n", s_file);
	exit(-1);
  }
  setbuf(s_fd, io_buf);
  ast(s_fd, o_fd);
  exit(0);
}

ast(s_fd, o_fd)
FILE *s_fd, *o_fd;
{
  struct nlist symbol;
  int line_type;

  do_header();
  for (;;) {
	read_line(s_fd, buffer);
	line_type = transform_line(buffer, &symbol);
	if (line_type == WORTH_LESS) continue;
	if (line_type == LAST_LINE) break;
	save_line(o_fd, &symbol);
  }
  redo_header(o_fd);
}

read_line(fd, buffer)
FILE *fd;
char *buffer;
{
  int ch;
  char *buf1;

  buf1 = buffer;
  *buffer = '\n';
  ch = fgetc(fd);
  while (ch != '\n' && ch != EOF) {
	*buffer = ch;
	buffer++;
	ch = fgetc(fd);
  }
  if (ch == EOF)
	*buffer = '\0';
  else
	*buffer = '\n';
  buffer[1] = '\0';
}

transform_line(buffer, symbol)
char *buffer;
struct nlist *symbol;
{
  switch (*buffer) {
      case 'a':			/* absolute symbol */
	symbol->n_sclass = N_ABS;
	break;
      case 'A':	symbol->n_sclass = N_ABS | C_EXT;	break;
      case 'u':			/* undefined symbol */
	symbol->n_sclass = N_UNDF;
	break;
      case 'U':
	symbol->n_sclass = N_UNDF | C_EXT;
	break;

      case 't':			/* text symbol */
	symbol->n_sclass = N_TEXT;
	break;
      case 'T':
	symbol->n_sclass = N_TEXT | C_EXT;
	break;
      case 'd':
	symbol->n_sclass = N_DATA;
      case 'D':			/* data symbol */
	symbol->n_sclass = N_DATA | C_EXT;
	break;
      case 'b':
	symbol->n_sclass = N_BSS;
      case 'B':			/* bss symbol */
	symbol->n_sclass = N_BSS | C_EXT;
	break;
      case '\0':		/* reached end of file */
	return(LAST_LINE);
      default:			/* one of first two lines */
	return(WORTH_LESS);
  }

  if (buffer[1] != ' ') {
	fprintf(stderr, "illegal file format\n");
	exit(-1);
  }
  symbol->n_value = get_value(buffer + 2);

  if (buffer[6] != ' ') {
	fprintf(stderr, "illegal file format\n");
	exit(-1);
  }
  get_name(buffer + 7, symbol->n_name);
  return(0);			/* yeah, found a symbol */
}

save_line(fd, symbol)
FILE *fd;
struct nlist *symbol;
{
  if (!(symbol->n_sclass & C_EXT)) {	/* local symbol */
	if (x_flag) return;
	if (X_flag && symbol->n_name[0] == 'I') return;
	if (X_flag && symbol->n_name[0] == 'L') return;
  }
  if (fwrite(symbol, sizeof(struct nlist), 1, fd) != 1) {
	fprintf(stderr, "can't write %s\n", o_file);
	exit(-1);
  }
  nr_symbols++;
}

unsigned get_value(string)
char *string;
{
  unsigned value;
  int shift, bits;

  value = 0;
  for (shift = 0; shift < 16; shift += 4) {
	bits = get_bits(*string);
	value = (value << 4) | bits;
	string++;
  }
  return(value);
}

get_bits(ch)
char ch;
{
  if (ch >= '0' && ch <= '9') return(ch - '0');
  if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 10);
  if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 10);
  fprintf(stderr, "illegal file format\n");
  exit(-1);
}

get_name(str1, str2)
register char *str1, *str2;
{
  int count;

  for (count = 0; count < 8; count++) {
	if (*str1 == '\n') break;
	*str2++ = *str1++;
  }
  while (count < 8) {
	*str2++ = '\0';
	count++;
  }
}

do_header()
{
  int fd;

  fd = open(o_file, O_RDONLY);
  if (read(fd, &header, sizeof(struct exec)) != sizeof(struct exec)) {
	fprintf(stderr, "%s: no executable file\n", o_file);
	exit(-1);
  }
  if (BADMAG(header)) {
	fprintf(stderr, "%s: bad header\n", o_file);
	exit(-1);
  }
  if (header.a_syms != 0L) {
	fprintf(stderr, "%s: symbol table is installed\n", o_file);
	exit(-1);
  }
  fseek(o_fd, A_SYMPOS(header), 0);
  nr_symbols = 0;
  close(fd);
}

redo_header(fd)
FILE *fd;
{
  header.a_syms = (long) (nr_symbols * sizeof(struct nlist));
  fseek(fd, 0L, 0);
  if (fwrite(&header, sizeof(header), 1, fd) != 1) {
	fprintf(stderr, "%s: can't write\n", o_file);
	exit(-1);
  }
}
