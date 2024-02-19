/*  look - look up words in the dictionary	Author: Terrence W. Holm */


/*  look  [ -f ]  prefix[/suffix]  [ dictionary ]
 *
 *  Looks for all words in the on-line dictionary
 *  beginning with the specified <prefix> and ending
 *  with <suffix>.
 *
 *  Fold to lower case if "-f" given. Use the file
 *  "dictionary" or /usr/lib/dictionary.
 *
 ******************************************************
 *
 *  This command was written for MINIX, and is slightly
 *  different than the UNIX look(1). First of all, the
 *  list of words is in a different place. Second, these
 *  words are not sorted by -df. And finally, the
 *  ``suffix'' option was added to limit the output of
 *  the multiple variations of each word as contained
 *  in the MINIX dictionary.
 */

#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef UNIX
#define  WORDS   "/usr/dict/words"
#else
#define  WORDS   "/usr/lib/dictionary"
#endif

#define  MAX_WORD_LENGTH   80	/* including '\0'  */


main(argc, argv)
int argc;
char *argv[];

{
  int fold = 0;
  char *prefix;
  char *suffix;
  char *word_file = WORDS;

  FILE *words;
  long head = 0;
  long tail;
  long mid_point;
  char word[MAX_WORD_LENGTH];
  char unfolded_word[MAX_WORD_LENGTH];
  int cmp;


  /* Check the arguments: fold, prefix, suffix and word_file.  */

  if (argc > 1 && strcmp(argv[1], "-f") == 0) {
	fold = 1;
	--argc;
	++argv;
  }
  if (argc < 2 || argc > 3) {
	fprintf(stderr, "Usage: %s [-f] prefix[/suffix] [dictionary]\n", argv[0]);
	exit(1);
  }
  prefix = argv[1];

  if ((suffix = strchr(prefix, '/')) == NULL)
	suffix = "";
  else
	*suffix++ = '\0';

  if (fold) {
	Fold(prefix);
	Fold(suffix);
  }
  if (argc == 3) word_file = argv[2];


  /* Open the word file, and find how big it is.  */
  if ((words = fopen(word_file, "r")) == NULL) File_Error(word_file);
  if (fseek(words, 0L, SEEK_END) != 0) File_Error(word_file);
  tail = ftell(words);

  /* Use a binary search on the word file to find a 512 byte	 */
  /* Window containing the first word starting with "prefix".	 */
  while (head + 512 < tail) {
	mid_point = (head + tail) / 2;
	if (fseek(words, mid_point, SEEK_SET) != 0) File_Error(word_file);

	/* Skip the partial word we seeked into.  */
	fgets(word, MAX_WORD_LENGTH, words);
	if (fgets(word, MAX_WORD_LENGTH, words) == NULL)
		File_Error(word_file);
	word[strlen(word) - 1] = '\0';	/* remove \n  */
	strcpy(unfolded_word, word);
	if (fold) Fold(word);
	cmp = strcmp(prefix, word);
	if (cmp == 0) {
		printf("%s\n", unfolded_word);
		head = ftell(words);
		break;
	}
	if (cmp < 0)
		tail = mid_point;
	else
		head = ftell(words);
  }

  fseek(words, head, SEEK_SET);



  {
	/* Print out all the words starting with "prefix".  */

	int prefix_length = strlen(prefix);
	int suffix_length = strlen(suffix);
	int word_length;


	while (fgets(word, MAX_WORD_LENGTH, words) != NULL) {
		word_length = strlen(word) - 1;
		word[word_length] = '\0';	/* remove \n  */
		strcpy(unfolded_word, word);
		if (fold) Fold(word);
		cmp = strncmp(prefix, word, prefix_length);
		if (cmp < 0) break;
		if (cmp == 0)
			if (suffix_length == 0 || word_length >= suffix_length
			    && strcmp(suffix, word + word_length - suffix_length) == 0)
				printf("%s\n", unfolded_word);
	}
  }

  fclose(words);

  exit(0);
}



Fold(str)
char *str;

{
  while (*str) {
	if (isupper(*str)) *str = *str - 'A' + 'a';
	str++;
  }
}



File_Error(word_file)
char *word_file;

{
  perror(word_file);
  exit(1);
}
