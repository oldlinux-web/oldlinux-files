typedef union {
	char *str;
	box *b;
	pile_box *pb;
	matrix_box *mb;
	int n;
	column *col;
} YYSTYPE;
#define	OVER	258
#define	SMALLOVER	259
#define	SQRT	260
#define	SUB	261
#define	SUP	262
#define	LPILE	263
#define	RPILE	264
#define	CPILE	265
#define	PILE	266
#define	LEFT	267
#define	RIGHT	268
#define	TO	269
#define	FROM	270
#define	SIZE	271
#define	FONT	272
#define	ROMAN	273
#define	BOLD	274
#define	ITALIC	275
#define	FAT	276
#define	ACCENT	277
#define	BAR	278
#define	UNDER	279
#define	ABOVE	280
#define	TEXT	281
#define	QUOTED_TEXT	282
#define	FWD	283
#define	BACK	284
#define	DOWN	285
#define	UP	286
#define	MATRIX	287
#define	COL	288
#define	LCOL	289
#define	RCOL	290
#define	CCOL	291
#define	MARK	292
#define	LINEUP	293
#define	TYPE	294
#define	VCENTER	295
#define	PRIME	296
#define	SPLIT	297
#define	NOSPLIT	298
#define	UACCENT	299
#define	SPECIAL	300
#define	SPACE	301
#define	GFONT	302
#define	GSIZE	303
#define	DEFINE	304
#define	NDEFINE	305
#define	TDEFINE	306
#define	SDEFINE	307
#define	UNDEF	308
#define	IFDEF	309
#define	INCLUDE	310
#define	DELIM	311
#define	CHARTYPE	312
#define	SET	313
#define	GRFONT	314
#define	GBFONT	315


extern YYSTYPE yylval;
