/* parse.y */

/*
 * Adapted from rc grammar, v10 manuals, volume 2.
 */

%{
#include "stddef.h"
#include "node.h"
#include "lex.h"
#include "tree.h"
#include "heredoc.h"
#include "parse.h"
#include "utils.h"
#undef NULL
#define NULL 0
#undef lint
#define lint		/* hush up gcc -Wall, leave out the dumb sccsid's. */
static Node *star, *nolist;
Node *parsetree;	/* not using yylval because bison declares it as an auto */
%}

%term ANDAND BACKBACK BANG CASE COUNT DUP ELSE END FLAT FN FOR IF IN
%term OROR PIPE REDIR SUB SUBSHELL SWITCH TWIDDLE WHILE WORD HUH

%left WHILE ')' ELSE
%left ANDAND OROR '\n'
%left BANG SUBSHELL
%left PIPE
%right '$' 
%left SUB
/*
*/

%union {
	struct Node *node;
	struct Redir redir;
	struct Pipe pipe;
	struct Dup dup;
	struct Word word;
	char *keyword;
}

%type <redir> REDIR
%type <pipe> PIPE
%type <dup> DUP
%type <word> WORD
%type <keyword> keyword
%type <node> assign body brace case cbody cmd cmdsa cmdsan comword epilog
	     first line paren redir sword simple iftail word words

%start rc

%%

rc	: line end		{ parsetree = $1; YYACCEPT; }
	| error end		{ yyerrok; parsetree = NULL; YYABORT; }

/* an rc line may end in end-of-file as well as newline, e.g., rc -c 'ls' */
end	: END	/* EOF */	{ if (!heredoc(1)) YYABORT; } /* flag error if there is a heredoc in the queue */
	| '\n'			{ if (!heredoc(0)) YYABORT; } /* get heredoc on \n */

/* a cmdsa is a command followed by ampersand or newline (used in "line" and "body") */
cmdsa	: cmd ';'
	| cmd '&'		{ $$ = ($1 != NULL ? newnode(NOWAIT,$1) : $1); }

/* a line is a single command, or a command terminated by ; or & followed by a line (recursive) */
line	: cmd
	| cmdsa line		{ $$ = ($1 != NULL ? newnode(BODY,$1,$2) : $2); }

/* a body is like a line, only commands may also be terminated by newline */
body	: cmd
	| cmdsan body		{ $$ = ($1 == NULL ? $2 : $2 == NULL ? $1 : newnode(BODY,$1,$2)); }

cmdsan	: cmdsa
	| cmd '\n'		{ $$ = $1; if (!heredoc(0)) YYABORT; } /* get h.d. on \n */

brace	: '{' body '}'		{ $$ = $2; }

paren	: '(' body ')'		{ $$ = $2; }

assign	: first '=' word	{ $$ = newnode(ASSIGN,$1,$3); }

epilog	:			{ $$ = NULL; }
	| redir epilog		{ $$ = newnode(EPILOG,$1,$2); }

/* a redirection is a dup (e.g., >[1=2]) or a file redirection. (e.g., > /dev/null) */
redir	: DUP			{ $$ = newnode(rDUP,$1.type,$1.left,$1.right); }
	| REDIR word		{ $$ = newnode(rREDIR,$1.type,$1.fd,$2);
				  if ($1.type == HEREDOC && !qdoc($2, $$)) YYABORT; /* queue heredocs up */
				}

case	: CASE words ';'	 		{ $$ = newnode(rCASE, $2); }
	| CASE words '\n'	 		{ $$ = newnode(rCASE, $2); }

cbody	: cmd					{ $$ = newnode(CBODY, $1, NULL); }
	| case cbody				{ $$ = newnode(CBODY, $1, $2); }
	| cmdsan cbody				{ $$ = newnode(CBODY, $1, $2); }

iftail	: cmd		%prec ELSE
	| brace ELSE optnl cmd			{ $$ = newnode(rELSE,$1,$4); }

cmd	: /* empty */	%prec WHILE		{ $$ = NULL; }
	| simple
	| brace epilog				{ $$ = newnode(BRACE,$1,$2); }
	| IF paren optnl iftail			{ $$ = newnode(rIF,$2,$4); }
	| FOR '(' word IN words ')' optnl cmd	{ $$ = newnode(FORIN,$3,$5,$8); }
	| FOR '(' word ')' optnl cmd		{ $$ = newnode(FORIN,$3,star,$6); }
	| WHILE paren optnl cmd			{ $$ = newnode(rWHILE,$2,$4); }
	| SWITCH '(' word ')' optnl '{' cbody '}' { $$ = newnode(rSWITCH,$3,$7); }
	| TWIDDLE optcaret word words		{ $$ = newnode(MATCH,$3,$4); }
	| cmd ANDAND optnl cmd			{ $$ = newnode(rANDAND,$1,$4); }
	| cmd OROR optnl cmd			{ $$ = newnode(rOROR,$1,$4); }
 	| cmd PIPE optnl cmd			{ $$ = newnode(rPIPE,$2.left,$2.right,$1,$4); }
	| redir cmd	%prec BANG		{ $$ = ($2 != NULL ? newnode(PRE,$1,$2) : $1); }
	| assign cmd	%prec BANG		{ $$ = ($2 != NULL ? newnode(PRE,$1,$2) : $1); }
	| BANG optcaret cmd			{ $$ = newnode(rBANG,$3); }
	| SUBSHELL optcaret cmd			{ $$ = newnode(rSUBSHELL,$3); }
	| FN words brace			{ $$ = newnode(NEWFN,$2,$3); }
	| FN words				{ $$ = newnode(RMFN,$2); }

optcaret : /* empty */
	| '^'

simple	: first
	| simple word			{ $$ = ($2 != NULL ? newnode(ARGS,$1,$2) : $1); }
	| simple redir			{ $$ = newnode(ARGS,$1,$2); }

first	: comword
	| first '^' sword		{ $$ = newnode(CONCAT,$1,$3); }

sword	: comword
	| keyword			{ $$ = newnode(rWORD,$1, NULL); }

word	: sword
	| word '^' sword		{ $$ = newnode(CONCAT,$1,$3); }

comword	: '$' sword			{ $$ = newnode(VAR,$2); }
	| '$' sword SUB words ')'	{ $$ = newnode(VARSUB,$2,$4); }
	| COUNT sword			{ $$ = newnode(rCOUNT,$2); }
	| FLAT sword			{ $$ = newnode(rFLAT, $2); }
	| '`' sword			{ $$ = newnode(BACKQ,nolist,$2); }
	| '`' brace			{ $$ = newnode(BACKQ,nolist,$2); }
	| BACKBACK word	brace		{ $$ = newnode(BACKQ,$2,$3); }
	| BACKBACK word	sword		{ $$ = newnode(BACKQ,$2,$3); }
	| '(' words ')'			{ $$ = $2; }
	| REDIR brace			{ $$ = newnode(NMPIPE,$1.type,$1.fd,$2); }
	| WORD				{ $$ = ($1.w[0] == '\'') ? newnode(QWORD, $1.w+1, NULL) : newnode(rWORD,$1.w, $1.m); }

keyword	: FOR		{ $$ = "for"; }
	| IN		{ $$ = "in"; }
	| WHILE		{ $$ = "while"; }
	| IF		{ $$ = "if"; }
	| SWITCH	{ $$ = "switch"; }
	| FN		{ $$ = "fn"; }
	| ELSE		{ $$ = "else"; }
	| CASE		{ $$ = "case"; }
	| TWIDDLE	{ $$ = "~"; }
	| BANG		{ $$ = "!"; }
	| SUBSHELL	{ $$ = "@"; }

words	:		{ $$ = NULL; }
	| words word	{ $$ = ($1 != NULL ? ($2 != NULL ? newnode(LAPPEND,$1,$2) : $1) : $2); }

optnl	: /* empty */
	| optnl '\n'

%%

void initparse() {
	star = treecpy(newnode(VAR,newnode(rWORD,"*",NULL)), ealloc);
	nolist = treecpy(newnode(VAR,newnode(rWORD,"ifs",NULL)), ealloc);
}

