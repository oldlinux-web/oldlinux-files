/* XRF.PL :  Prolog Cross-Reference Program 

						Dave Bowen
						Updated: 8 March 84

						Jan Wielemaker
						Updated: 31 Januari 1985

	This is the main module of XREF.
*/

/*
* This program is extended with a help facility and an indication from which
* predicate and file an exported predicate is called.
* I also also addapted the program for use with on the UvApsy VAX.
* Jan Wielemaker, 10/12/'84
*/

/*
* 'system' is renamed in 'sys_pred' because system is a system predicate
* itself.
* Jan Wielemaker, 7/12/84
*/

/*
* This version uses the Prolog 1.5 primitive 'system/1' to determine whether
* a predicate is a system predicates. It is still possible to add extra 
* system predicates by putting 'sys_pred/1' facts in a definition file.
*/

/*  
    updates: (Jan Wielemaker

    4/11/85: Included files are processed automatically now.
    5/11/85: Library fils are processed if they are not system files.
*/

% Compiler declarations %
/*
 :- public go/0, start/0.		% User entry.
 :- public xrf/1, indirect/1.		% These are public for internal Call.
*/
 :- op(1050,xfy,->).
 :- op(500,fy,@).
/*
 :- mode caller(+,+,+,+).
 :- mode crecord(+,+).
 :- mode definition(+,+).
 :- mode do_file(+,+).
 :- mode exhaust(+). 
 :- mode getfrom(+).
 :- mode goal(?,-). 
 :- mode goal0(+,-).
 :- mode has_ext(+,+).
 :- mode head(+,+,-,-).  
 :- mode indirect(+).    
 :- mode mark_interpreted(+).
 :- mode process(+,+).   
 :- mode reply(+).  
 :- mode widen(?,-).
 :- mode xrf(+).
*/

/******************************************************************************

Data

   $caller(Called_Predicate, Calling_Functor, Calling_Arity, Where_Defined)
			  Held on key: Called_Predicate.
			  $caller(P,F,N,I) means P is called by F/N in file I.
   $defn(File, Predicate)
			  Held on key: Predicate.
			  $defn(I,P) means P is defined in I.
   $file(File)
			  Held on key: $file(_).
			  $file(F) means F is a file.
   Predicate
			  Held on key: $pred
			  pred(G,M) means G/M was defined OR used.

*****************************************************************************/


				/* Top level */
go :-	find_def_file(File),
	load(File),
	start.

find_def_file('xref.def') :-
	exists('xref.def'), !.
find_def_file(File) :-
	lib('xref.d/xref.def',File).

xref :-
	nl,write('Prolog Cross Reference Program'),nl,nl,
	write('Reply ? <return> if you want help.'),nl,
	write('The manual is in /usr/local/lib/prolog/xref.d/READ_ME.'),nl,
	recorda($define,width(78),_),	% default linewidth
	repeat,
	    nl, write('Next file: '), ttyflush,
	    readtonl(String),
	    reply(String), !.

				/* Check for termination of input: just 
				   <newline> typed? */
reply([]) :- !,
	(collect ; true).	% Yes: go and start output phase
reply(String) :-
	getfrom(String), !,	% Something input: what is it?
	fail.			% Go back to repeat

				/* Prolog, indirect or definition file? */

getfrom([63|_]) :- !,			% print help text (started with '?')
	help_file_in.
getfrom([64|S]) :- !,
	do_file(indirect,S).		% Indirect file introduced by '@'
getfrom([42|S]) :- !,
	do_file(load_file,S).		% Definition file introduced by '*'
getfrom(S) :- 
	has_ext(S,".ccl"), !,		% Indirect file indicated by ".ccl"?
	do_file(indirect,S).
getfrom(S) :- 
	has_ext(S,".def"), !,		% Definition file indicated by ".def"?
	do_file(load_file,S).
getfrom(S) :-				% Must be Prolog file for cross ref
	do_file(xrf,S).

				/* Takes two character lists as arguments, and
				   tests whether the 1st ends with the 2nd */
has_ext(Ext,Ext) :- !.
has_ext([_|L],Ext) :- has_ext(L,Ext).

				/* Open up the file, call processing procedure,
				   and close file again. Calls indirect(File)
				   load_file(File) or xrf(File). */
do_file( Predicate,String ) :-
	name( File,String ),		% Convert char list to atom (File)
	seeing( Old_file ),		% Save currently open file
	see_chek( File ),		% Open required file (may fail)
	P =.. [Predicate,File],		% Construct call to reqd procedure
	call( P ),			% Call it
	seen,				% Close file
	see(Old_file).			% Re-open original file
do_file( _,_ ).				% Always succeed

				/* Get file names from indirect file */
indirect( F ) :- 
	readtonl(S),			% Read a line, fail at end_of_file(^Z)
	( ((S="");(S=[37|_]))		% Ignore blank lines and % ... lines
	;   write('File: '),		% (S is char list)
	    writes(S), nl,		% Echo on terminal
	    getfrom(S)			% Process (may be indirect or def)
	), !,
	indirect( F ).			% Loop to get next line
indirect( F ) :-
	nl,				% Tell user when indirect file finished
	write('Indirect file '),
	write(F),
	write(' processed'), nl.

				/* Cross reference processing for a particular
				   file F */
xrf( library( Lib ) ) :-
	lib( Lib,File ),
	seeing( Old ),
	see_chek( File ),
	recordz('$file'(_),'$file'(File),_),	% record F under $file
	exhaust( File,library ), !,
	seen,
	see( Old ), !.
xrf( File ) :-
	seeing( Old ),
	see_chek( File ),
	recordz('$file'(_),'$file'(File),_),	% record F under $file
	exhaust( File,file),			% go through F term by term
	seen,
	see( Old ), !.

				/* Process each clause, T, in file F */
exhaust( File,library ) :-
	repeat,
	    read(T),			% Read a clause
	    expand_term(T,T1),		% Pre-translation of grammar rules
	    lib_process(T1,File),	% Cross-ref processing
	T = end_of_file, !.
exhaust( File,file ) :- 
	repeat,				% Iterate till end_of_file
	    read(T),			% Read a clause
	    expand_term(T,T1),		% Pre-translation of grammar rules
	    process(T1,File),		% Cross-ref processing
	T = end_of_file, !.

				/* Process clause (1st arg). 2nd arg is file */

lib_process( (P:-_),File ) :- !,
	lib_head( P,File ).
lib_process( (:-_),_ ) :- !, fail.
lib_process( (?-_),_ ) :- !, fail.	
lib_process( end_of_file,_ ) :- !.
lib_process( P,File ) :-
	lib_head( P,File ).

process((P:-Q),File) :- !, 		% Non-unit clause
	head(P,File,F,N), !,
	goal(Q,G),			% Process successive goals by 
	caller(G,F,N,File).		% Backtracking.
process((:-G),File) :- !,		% Command
	directive(G,File).
process((?-G),File) :- !,		% question, ditto
	directive(G,File).
process(end_of_file,_) :- !.		% eof so succeed and exit repeat loop
process(P,File) :- 			% unit clause
	head(P,File,_,_).

directive( Consult,_ ) :-		% Consult. Process consulted files as well
	is_list( Consult ),
	indir( Consult,consults ), !.
directive( consult(Files),_ ) :-
	indir( Files, consults ), !.
directive( reconsult(Files),_ ) :-
	indir( Files,reconsults ), !.
directive( include(Files),_ ) :-
	indir( Files,includes ), !.
directive( ensure_loaded(Files),_ ) :-	% Quintus compatibility
	indir( Files,includes ), !.
directive( compile(Files),_ ) :-	% Quintus compatibility
	indir( Files,includes ), !.
directive( lib(Files),_ ) :-
	lib_indir( Files ), !.
directive( dynamic( Predicates ),_ ) :- !,
	mark_dynamic( Predicates ).

				% Process directive as if clause for user/0
directive(Goals,File) :- 
	goal(Goals,Goal),
	caller(Goal,'<user>',0,File).

%	process included library files if they are not loaded with the system

lib_indir([]) :- !.
lib_indir([Hd|Tl]) :-
	(   lib(Hd, FileFound),
	    expand_file_name(FileFound, FullFile),
	    (   file_description(lib, FullFile, _)	% system file
	    |
		mxrf(FullFile, 'includes lib. file')
	    )
	|
	    true
	), !,
	lib_indir(Tl).

%	process list of consulted files

indir([], _) :- !.
indir(F, T) :- atom(F), !, mxrf(F, T).
indir([-Hd|Tl], T) :- mxrf(Hd, reconsults), indir(Tl, T).
indir([Hd|Tl], T) :- mxrf(Hd, consults), indir(Tl, T).
indir( library(F),T ) :- mxrf( library(F),T ).

mxrf(Hd, T) :- 
	write(T), write(' file '),
	write(Hd),
	write(' (processing)'),nl,
	xrf( Hd ), !.

/*  dynamic predicates are to be found in the database.
*/

mark_dynamic( (A,B) ) :-
	mark_dynamic( A ),
	mark_dynamic( B ), !.
mark_dynamic( Pred/Arity ) :-
	functor( F,Pred,Arity ),
	note( known(F,'<database>') ), !.

/*  record the fact that P is defined on library file F.
*/

lib_head( P,File ) :-
	functor( P,F,N ),
	functor( G,F,N ),
	crecord( G,'$library'( File ) ), !.

				/* Record the fact that P is a predicate & that
				   it is defined in file I. Return principal
				   functor of P (F) & its arity (N). */
head(P,I,F,N) :-
	functor(P,F,N),			% P has name F and arity N
	functor(G,F,N),			% G is most general term F/N
	definition(G,I).		% Record F/N is pred & defined in I

				/* Fail if goal is a variable. */
goal(G,_) :- var(G), !, fail.
goal(G,G1) :- goal0(G,G1).

				/* Returns most general term having the
				   principal functor & arity of each goal in 
				   the clause (successively on backtracking).
				   Ignores system predicates. */
goal0((G,_),G1) :-
   goal(G,G1).
goal0((_,G),G1) :- !,
   goal(G,G1).
goal0((G;_),G1) :-
   goal(G,G1).
goal0((_;G),G1) :- !,
   goal(G,G1).
goal0(op(P,T,O),_) :- !,
   op(P,T,O), 
   fail.
goal0(G1,G2) :- 
   recorded(G1,'$applies'(G1,P),_),
   widen(P,P1),
   goal(P1,G2).
goal0(G,_) :- 
   (   system(G) 		%   this is a prolog1.5 primitive
   ;   recorded(G,'$sys_pred',_)
   ), !,
   fail.
goal0(G,G).

				/* Record that P is a predicate and that it is
				   defined in file I */
definition(P,_) :-
	(   system(P)		% But not if P is a system predicate
	;   recorded(P,'$sys_pred',_)
	), !,
	warn(P,'already defined as a system predicate'), fail.
definition(P,I) :-
	crecord('$pred',P),
	crecord(P,'$defn'(I,P)).

				/* Record that P is a predicate called by
				   F/N in file I */
caller(P,F,N,I) :- 
	functor(P,Pf,Pn),
	functor(P1,Pf,Pn),
	crecord('$pred',P1),
	crecord(P1,'$caller'(P1,F,N,I)).

				/* Record term Q on key P unless already
				   recorded. */
crecord(P,Q) :- recorded(P,Q,_), !.
crecord(P,Q) :- recordz(P,Q,_), !.

				/* Increase arity of predicate by specified
				   amount */
widen(A+_,_) :-
	var(A), !, fail.	% NB also covers variable as first arg
widen(A+Offset,A1) :- !,
	functor(A,F,N1),
	N2 is N1+Offset,
	functor(A1,F,N2).
widen(A,A).

help_file_in :-
    write('type <filename> <cr>, or'),nl,
    write('     @<filename> <cr> for indirect file, or'),nl,
    write('     <filename, ending with ".ccl"> <cr> for indirect file, or'),nl,
    write('     *<filename> <cr> for definitions, or'),nl,
    write('     <filename, ending with ".def"> for definitions, or'),nl,
    write('     single <cr> to terminate the input phase'),nl.

