typedef union {
	char	 *s_value;
	char	  c_value;
	int	  i_value;
	arg_list *a_value;
       } YYSTYPE;
#define	NEWLINE	258
#define	AND	259
#define	OR	260
#define	NOT	261
#define	STRING	262
#define	NAME	263
#define	NUMBER	264
#define	MUL_OP	265
#define	ASSIGN_OP	266
#define	REL_OP	267
#define	INCR_DECR	268
#define	Define	269
#define	Break	270
#define	Quit	271
#define	Length	272
#define	Return	273
#define	For	274
#define	If	275
#define	While	276
#define	Sqrt	277
#define	Else	278
#define	Scale	279
#define	Ibase	280
#define	Obase	281
#define	Auto	282
#define	Read	283
#define	Warranty	284
#define	Halt	285
#define	Last	286
#define	Continue	287
#define	Print	288
#define	Limits	289
#define	UNARY_MINUS	290


extern YYSTYPE yylval;
