/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -p -j1 -g -o -t -N is_reserved_word -k1,4,7,$ ./gplus.gperf  */
/* Command-line: gperf -p -j1 -g -o -t -N is_reserved_word -k1,4,$,7 gplus.gperf  */
struct resword { char *name; short token; enum rid rid;};

#define TOTAL_KEYWORDS 81
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 13
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 199
/* maximum key range = 196, duplicates = 0 */

#ifdef __GNUC__
inline
#endif
static unsigned int
hash (str, len)
     register char *str;
     register int unsigned len;
{
  static unsigned char asso_values[] =
    {
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200, 200, 200, 200, 200, 200,
     200, 200, 200, 200, 200,   0, 200,  35,   1,  52,
      53,   0,   5,  11,  44,  21, 200,  13,  35,  36,
      21,   0,   9, 200,  30,   0,   8,  49,   5,  64,
       0,  34, 200, 200, 200, 200, 200, 200,
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 7:
        hval += asso_values[str[6]];
      case 6:
      case 5:
      case 4:
        hval += asso_values[str[3]];
      case 3:
      case 2:
      case 1:
        hval += asso_values[str[0]];
    }
  return hval + asso_values[str[len - 1]];
}

#ifdef __GNUC__
inline
#endif
struct resword *
is_reserved_word (str, len)
     register char *str;
     register unsigned int len;
{
  static struct resword wordlist[] =
    {
      {"",}, {"",}, {"",}, {"",}, 
      {"else",  ELSE, NORID,},
      {"",}, {"",}, 
      {"__asm__",  ASM_KEYWORD, NORID},
      {"",}, {"",}, 
      {"__headof__",  HEADOF, NORID},
      {"sizeof",  SIZEOF, NORID,},
      {"this",  THIS, NORID,},
      {"__headof",  HEADOF, NORID},
      {"except",  EXCEPT, NORID		/* Extension */,},
      {"goto",  GOTO, NORID,},
      {"",}, 
      {"__const__",  TYPE_QUAL, RID_CONST},
      {"__volatile",  TYPE_QUAL, RID_VOLATILE},
      {"typeof",  TYPEOF, NORID,},
      {"__volatile__",  TYPE_QUAL, RID_VOLATILE},
      {"private",  VISSPEC, RID_PRIVATE,},
      {"",}, 
      {"__const",  TYPE_QUAL, RID_CONST},
      {"",}, 
      {"typedef",  SCSPEC, RID_TYPEDEF,},
      {"",}, 
      {"extern",  SCSPEC, RID_EXTERN,},
      {"if",  IF, NORID,},
      {"",}, {"",}, 
      {"__signed__",  TYPESPEC, RID_SIGNED},
      {"int",  TYPESPEC, RID_INT,},
      {"template",  TEMPLATE, NORID,},
      {"__extension__",  EXTENSION, NORID},
      {"raise",  RAISE, NORID		/* Extension */,},
      {"raises",  RAISES, NORID		/* Extension */,},
      {"",}, 
      {"for",  FOR, NORID,},
      {"auto",  SCSPEC, RID_AUTO,},
      {"__attribute",  ATTRIBUTE, NORID},
      {"__asm",  ASM_KEYWORD, NORID},
      {"__attribute__",  ATTRIBUTE, NORID},
      {"short",  TYPESPEC, RID_SHORT,},
      {"__typeof__",  TYPEOF, NORID},
      {"try",  TRY, NORID			/* Extension */,},
      {"__classof__",  CLASSOF, NORID},
      {"__typeof",  TYPEOF, NORID},
      {"inline",  SCSPEC, RID_INLINE,},
      {"__classof",  CLASSOF, NORID},
      {"__inline",  SCSPEC, RID_INLINE},
      {"exception",  AGGR, RID_EXCEPTION	/* Extension */,},
      {"__inline__",  SCSPEC, RID_INLINE},
      {"float",  TYPESPEC, RID_FLOAT,},
      {"break",  BREAK, NORID,},
      {"do",  DO, NORID,},
      {"case",  CASE, NORID,},
      {"class",  AGGR, RID_CLASS,},
      {"switch",  SWITCH, NORID,},
      {"delete",  DELETE, NORID,},
      {"double",  TYPESPEC, RID_DOUBLE,},
      {"long",  TYPESPEC, RID_LONG,},
      {"",}, 
      {"struct",  AGGR, RID_RECORD,},
      {"friend",  SCSPEC, RID_FRIEND,},
      {"const",  TYPE_QUAL, RID_CONST,},
      {"static",  SCSPEC, RID_STATIC,},
      {"__alignof__",  ALIGNOF, NORID},
      {"operator",  OPERATOR, NORID,},
      {"classof",  CLASSOF, NORID,},
      {"__alignof",  ALIGNOF, NORID},
      {"",}, 
      {"reraise",  RERAISE, NORID		/* Extension */,},
      {"all",  ALL, NORID			/* Extension */,},
      {"asm",  ASM_KEYWORD, NORID,},
      {"union",  AGGR, RID_UNION,},
      {"enum",  ENUM, NORID,},
      {"throw",  THROW, NORID		/* Extension */,},
      {"",}, 
      {"__label__",  LABEL, NORID},
      {"signed",  TYPESPEC, RID_SIGNED,},
      {"",}, 
      {"__signed",  TYPESPEC, RID_SIGNED},
      {"volatile",  TYPE_QUAL, RID_VOLATILE,},
      {"",}, {"",}, {"",}, 
      {"protected",  VISSPEC, RID_PROTECTED,},
      {"new",  NEW, NORID,},
      {"register",  SCSPEC, RID_REGISTER,},
      {"virtual",  SCSPEC, RID_VIRTUAL,},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, 
      {"public",  VISSPEC, RID_PUBLIC,},
      {"",}, 
      {"while",  WHILE, NORID,},
      {"",}, 
      {"return",  RETURN, NORID,},
      {"",}, 
      {"headof",  HEADOF, NORID,},
      {"",}, {"",}, 
      {"default",  DEFAULT, NORID,},
      {"",}, {"",}, {"",}, 
      {"void",  TYPESPEC, RID_VOID,},
      {"char",  TYPESPEC, RID_CHAR,},
      {"continue",  CONTINUE, NORID,},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"overload",  OVERLOAD, NORID,},
      {"",}, {"",}, {"",}, {"",}, 
      {"unsigned",  TYPESPEC, RID_UNSIGNED,},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, 
      {"catch",  CATCH, NORID,},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      
      {"dynamic",  DYNAMIC, NORID,},
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register char *s = wordlist[key].name;

          if (*s == *str && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
