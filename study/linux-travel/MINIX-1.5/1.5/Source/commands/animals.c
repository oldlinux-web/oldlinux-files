/*  animals - guessing game	Authors: Terrence W. Holm & Edwin L. Froese */

#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sgtty.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

#define  ANIMALS	"/usr/lib/animals"
#define  DEFAULT_ANIMAL	"beaver"
#define  MAX_NODES	999	/* Enough for 500 animals  */
#define  MAX_LINE	90

void Abort();
char *Get_Animal();
char *Get_Question();
char *A_or_An();
char *Alloc();

struct node {
  int question;
  char *text;
  int yes;
  int no;
} animals[MAX_NODES];

int count = 0;
struct sgttyb old_tty_mode;


main(argc, argv)
int argc;
char *argv[];

{
  char *animal_file = ANIMALS;

  if (argc > 2) {
	fprintf(stderr, "Usage:  %s  [ data_base ]\n", argv[0]);
	exit(1);
  }
  if (argc == 2) animal_file = argv[1];

  ioctl(0, TIOCGETP, &old_tty_mode);

  signal(SIGINT, Abort);
  signal(SIGQUIT, Abort);

  if (access(animal_file, R_OK) == 0)
	Read_Animals(animal_file);
  else {
	animals[0].question = 0;
	animals[0].text = DEFAULT_ANIMAL;
	count = 1;
  }

  while (Ask("\nAre you thinking of an animal?")) {
	int i = 0;

	while (1) {
		if (animals[i].question) {
			if (Ask(animals[i].text))
				i = animals[i].yes;
			else
				i = animals[i].no;
		} else {
			printf("Were you thinking of %s %s",
			 A_or_An(animals[i].text), animals[i].text);

			if (Ask("?")) printf("I knew it!\n");

			else {
				/* Insert a new question and animal name  */

				if (count + 2 > MAX_NODES)
					Error("Too many animal names");

				animals[count].question = 0;
				animals[count].text = animals[i].text;
				++count;

				animals[count].question = 0;
				printf("What animal were you thinking of? ");
				animals[count].text = Get_Animal();
				++count;

				animals[i].question = 1;
				printf("What question would distinguish %s %s from\n%s %s? ",
				       A_or_An(animals[count - 2].text), animals[count - 2].text,
				       A_or_An(animals[count - 1].text), animals[count - 1].text);

				animals[i].text = Get_Question();

				printf("For %s %s, the answer would be",
				       A_or_An(animals[count - 1].text), animals[count - 1].text);

				if (Ask("?")) {
					animals[i].yes = count - 1;
					animals[i].no = count - 2;
				} else {
					animals[i].yes = count - 2;
					animals[i].no = count - 1;
				}
			}

			break;
		}
	}			/* End while ( 1 )  */

  }


  ioctl(0, TIOCSETP, &old_tty_mode);

  printf("\nThank you for playing \"animals\".\n");
  printf("The animal data base is now being updated.\n");

  Write_Animals(animal_file);

  sleep(1);
  printf("\nBye.\n");

  exit(0);
}


/*  Reading and writing the animal data base */


Read_Animals(animal_file)
char *animal_file;

{
  FILE *f;
  char buffer[MAX_LINE];

  if ((f = fopen(animal_file, "r")) == NULL)
	Error("Can not open animal data base");

  while (fgets(buffer, MAX_LINE, f) != NULL) {
	int string_length;
	char *string;

	buffer[strlen(buffer) - 1] = '\0';

	swab(buffer, buffer, strlen(buffer));

	if (buffer[0] == 'q') {
		char *end = strchr(buffer, '?');
		string_length = end - buffer;
		animals[count].question = 1;
		sscanf(end+1, "%d:%d",&animals[count].yes,&animals[count].no);
	} else {
		animals[count].question = 0;
		string_length = strlen(buffer) - 1;
	}

	string = Alloc(string_length + 1);

	string[0] = '\0';
	strncat(string, buffer + 1, string_length);

	animals[count].text = string;

	count++;
  }

  fclose(f);
}


Write_Animals(animal_file)
char *animal_file;

{
  FILE *f;
  int i;
  char buffer[MAX_LINE];

  if ((f = fopen(animal_file, "w")) == NULL)
	Error("Can not write animal data base");

  for (i = 0; i < count; ++i) {
	if (animals[i].question) sprintf(buffer, "q%s%d:%d", animals[i].text,
			animals[i].yes, animals[i].no);
	else
		sprintf(buffer, "a%s", animals[i].text);

	/* Make the data base a bit difficult to read  */
	swab(buffer, buffer, strlen(buffer));
	fprintf(f, "%s\n", buffer);
  }

  fclose(f);
  chmod(animal_file, 0666);
}


/*  Reading data from the user */
int Ask(question)
char *question;
{
  struct sgttyb new_tty_mode;
  int response;

  new_tty_mode = old_tty_mode;
  new_tty_mode.sg_flags |= CBREAK;
  ioctl(0, TIOCSETP, &new_tty_mode);

  printf("%s ", question);

  while ((response = getchar()) != 'y' && response != 'n')
	printf("\n%s [yn]?", question);
  putchar('\n');
  ioctl(0, TIOCSETP, &old_tty_mode);
  if (response == 'y')
	return(1);
  else
	return(0);
}


char *Get_Animal()
{
  char s[MAX_LINE];
  char *text;
  int text_length;

  fgets(s, MAX_LINE, stdin);
  text_length = strlen(s);
  text = Alloc(text_length);
  text[0] = '\0';
  strncat(text, s, text_length - 1);
  return(text);
}


char *Get_Question()
{
  char s[MAX_LINE];
  char *end;
  char *text;

  fgets(s, MAX_LINE, stdin);

  /* Capitalize the first letter  */
  if (islower(s[0])) s[0] = toupper(s[0]);

  /* Make sure the question ends with a '?'  */
  if ((end = strchr(s, '?')) == NULL)
	s[strlen(s) - 1] = '?';
  else
	end[1] = '\0';

  text = Alloc(strlen(s) + 1);
  strcpy(text, s);
  return(text);
}


/*  Utility routines */
char *A_or_An(word)
char *word;
{
  if (strchr("aeiouAEIOU", word[0]) == NULL)
	return("a");
  else
	return("an");
}


char *Alloc(size)
int size;
{
  char *malloc();
  char *memory;

  if ((memory = malloc(size)) == NULL)
	Error("No room in memory for all the animals");

  return(memory);
}


void Abort()
{
  ioctl(0, TIOCSETP, &old_tty_mode);

  printf("\nThank you for playing \"animals\".\n");
  printf("Since you aborted, the animal data base will not be updated.\n");
  sleep(1);
  printf("\nBye.\n");
  exit(1);
}


Error(message)
char *message;
{
  ioctl(0, TIOCSETP, &old_tty_mode);
  fprintf(stderr, "Error: %s\n", message);
  exit(1);
}
