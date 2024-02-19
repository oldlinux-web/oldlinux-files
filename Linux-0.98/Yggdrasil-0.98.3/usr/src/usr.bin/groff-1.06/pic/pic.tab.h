typedef union {
	char *str;
	int n;
	double x;
	struct { double x, y; } pair;
	struct { double x; char *body; } if_data;
	struct { char *str; const char *filename; int lineno; } lstr;
	struct { double *v; int nv; int maxv; } dv;
	struct { double val; int is_multiplicative; } by;
	place pl;
	object *obj;
	corner crn;
	path *pth;
	object_spec *spec;
	saved_state *pstate;
	graphics_state state;
	object_type obtype;
} YYSTYPE;
#define	LABEL	258
#define	VARIABLE	259
#define	NUMBER	260
#define	TEXT	261
#define	COMMAND_LINE	262
#define	DELIMITED	263
#define	ORDINAL	264
#define	TH	265
#define	LEFT_ARROW_HEAD	266
#define	RIGHT_ARROW_HEAD	267
#define	DOUBLE_ARROW_HEAD	268
#define	LAST	269
#define	UP	270
#define	DOWN	271
#define	LEFT	272
#define	RIGHT	273
#define	BOX	274
#define	CIRCLE	275
#define	ELLIPSE	276
#define	ARC	277
#define	LINE	278
#define	ARROW	279
#define	MOVE	280
#define	SPLINE	281
#define	HEIGHT	282
#define	RADIUS	283
#define	WIDTH	284
#define	DIAMETER	285
#define	FROM	286
#define	TO	287
#define	AT	288
#define	WITH	289
#define	BY	290
#define	THEN	291
#define	DOTTED	292
#define	DASHED	293
#define	CHOP	294
#define	SAME	295
#define	INVISIBLE	296
#define	LJUST	297
#define	RJUST	298
#define	ABOVE	299
#define	BELOW	300
#define	OF	301
#define	THE	302
#define	WAY	303
#define	BETWEEN	304
#define	AND	305
#define	HERE	306
#define	DOT_N	307
#define	DOT_E	308
#define	DOT_W	309
#define	DOT_S	310
#define	DOT_NE	311
#define	DOT_SE	312
#define	DOT_NW	313
#define	DOT_SW	314
#define	DOT_C	315
#define	DOT_START	316
#define	DOT_END	317
#define	DOT_X	318
#define	DOT_Y	319
#define	DOT_HT	320
#define	DOT_WID	321
#define	DOT_RAD	322
#define	SIN	323
#define	COS	324
#define	ATAN2	325
#define	LOG	326
#define	EXP	327
#define	SQRT	328
#define	MAX	329
#define	MIN	330
#define	INT	331
#define	RAND	332
#define	COPY	333
#define	THRU	334
#define	TOP	335
#define	BOTTOM	336
#define	UPPER	337
#define	LOWER	338
#define	SH	339
#define	PRINT	340
#define	CW	341
#define	CCW	342
#define	FOR	343
#define	DO	344
#define	IF	345
#define	ELSE	346
#define	ANDAND	347
#define	OROR	348
#define	NOTEQUAL	349
#define	EQUALEQUAL	350
#define	LESSEQUAL	351
#define	GREATEREQUAL	352
#define	LEFT_CORNER	353
#define	RIGHT_CORNER	354
#define	CENTER	355
#define	END	356
#define	START	357
#define	RESET	358
#define	UNTIL	359
#define	PLOT	360
#define	THICKNESS	361
#define	FILL	362
#define	ALIGNED	363
#define	SPRINTF	364
#define	COMMAND	365
#define	DEFINE	366
#define	UNDEF	367


extern YYSTYPE yylval;
