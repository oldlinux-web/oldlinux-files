/*--------------------------------------------------------------------------

  match.c

  The match() routine recursively compares a string to a "pattern" (regular
  expression), returning TRUE if a match is found or FALSE if not.  This
  version is specifically for use with unzip.c:  it leaves the case (upper,
  lower, or mixed) of the string alone, but converts any uppercase characters
  in the pattern to lowercase if indicated by the global var pInfo->lcflag
  (which is to say, string is assumed to have been converted to lowercase
  already, if such was necessary).

  --------------------------------------------------------------------------

  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.

  --------------------------------------------------------------------------*/


#include "unzip.h"


/*******************/
/*  Match Defines  */
/*******************/

#define ASTERISK        '*'     /* The '*' metacharacter */
#define QUESTION        '?'     /* The '?' metacharacter */
#define BACK_SLASH      '\\'    /* The '\' metacharacter */
#define LEFT_BRACKET    '['     /* The '[' metacharacter */
#define RIGHT_BRACKET   ']'     /* The ']' metacharacter */
#define EOS             '\000'  /* end-of-string */

#define IS_OCTAL(ch)    (ch >= '0' && ch <= '7')



/********************/
/*  Match Typedefs  */
/********************/

typedef short int BOOLEAN;



/*************************************/
/*  Match Local Function Prototypes  */
/*************************************/

static BOOLEAN do_list __((register char *string, char *pattern));
static void list_parse __((char **patp, char *lowp, char *highp));
static char nextch __((char **patp));





/**********************/
/*  Function match()  */
/**********************/

int match(string, pattern)
char *string;
char *pattern;
{
    register int ismatch;

    ismatch = FALSE;
    switch (*pattern) {
    case ASTERISK:
        pattern++;
        do {
            ismatch = match(string, pattern);
        }
        while (!ismatch && *string++ != EOS);
        break;
    case QUESTION:
        if (*string != EOS)
            ismatch = match(++string, ++pattern);
        break;
    case EOS:
        if (*string == EOS)
            ismatch = TRUE;
        break;
    case LEFT_BRACKET:
        if (*string != EOS)
            ismatch = do_list(string, pattern);
        break;
    case BACK_SLASH:
        pattern++;
    default:
        if (*string == ((pInfo->lcflag && isupper(*pattern))?
            tolower(*pattern) : *pattern)) {
            string++;
            pattern++;
            ismatch = match(string, pattern);
        } else
            ismatch = FALSE;
        break;
    }
    return (ismatch);
}





/************************/
/*  Function do_list()  */
/************************/

static BOOLEAN do_list(string, pattern)
register char *string;
char *pattern;
{
    register BOOLEAN ismatch;
    register BOOLEAN if_found;
    register BOOLEAN if_not_found;
    auto char lower;
    auto char upper;

    pattern++;
    if (*pattern == '!') {
        if_found = FALSE;
        if_not_found = TRUE;
        pattern++;
    } else {
        if_found = TRUE;
        if_not_found = FALSE;
    }
    ismatch = if_not_found;
    while (*pattern != ']' && *pattern != EOS) {
        list_parse(&pattern, &lower, &upper);
        if (*string >= lower && *string <= upper) {
            ismatch = if_found;
            while (*pattern != ']' && *pattern != EOS)
                pattern++;
        }
    }

    if (*pattern++ != ']') {
        printf("Character class error\n");
        exit(1);
    } else if (ismatch)
        ismatch = match(++string, pattern);

    return (ismatch);
}





/***************************/
/*  Function list_parse()  */
/***************************/

static void list_parse(patp, lowp, highp)
char **patp;
char *lowp;
char *highp;
{
    *lowp = nextch(patp);
    if (**patp == '-') {
        (*patp)++;
        *highp = nextch(patp);
    } else
        *highp = *lowp;
}





/***********************/
/*  Function nextch()  */
/***********************/

static char nextch(patp)
char **patp;
{
    register char ch;
    register char chsum;
    register int count;

    ch = *(*patp)++;
    if (ch == '\\') {
        ch = *(*patp)++;
        if (IS_OCTAL(ch)) {
            chsum = 0;
            for (count = 0; count < 3 && IS_OCTAL(ch); count++) {
                chsum *= 8;
                chsum += ch - '0';
                ch = *(*patp)++;
            }
            (*patp)--;
            ch = chsum;
        }
    }
    return (ch);
}
