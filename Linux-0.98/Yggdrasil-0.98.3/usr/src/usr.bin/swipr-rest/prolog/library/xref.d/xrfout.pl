/* XRFOUT.PL : Output module for XREF

						Dave Bowen
						Updated: 20 November 83

						Jan Wielemaker
						Updated: 13 December 84
*/

/*
 :- mode do_imports(+).
 :- mode do_output(+).
 :- mode do_exports(+).
 :- mode format_out(+).
 :- mode f_back(+,+,-,+).
 :- mode f_front(+,+,+,+).
 :- mode f_output(+,+).
 :- mode f_write(+,+,-,+).
 :- mode f_writepred(+,+,+,-,+).
 :- mode getentries(+,-).
 :- mode getexts(+,-).
 :- mode imports1(+).
 :- mode lngth(+,-).
 :- mode makeroom(+,+,-,+).
 :- mode nl(+).
 :- mode number_length(+,-).
 :- mode one_imp_decl(+).
 :- mode one_pub_decl(+).
 :- mode outputdclrs(+).
 :- mode exports1(+).
 :- mode writes(+).
 :- mode write_callers(+,+,-,+).
 :- mode write_centre(+,+).
*/

				/* O/P cross-ref listing in reqd format.*/
do_output(L) :-
	recorded($crf,file(File,Chars),Ref),	%  Filename for cross ref. l.
	telling(Old),
	tell(File),
	format_out(L),				%  Output cross-reference list
	told,
	tell(Old),
	erase(Ref),
	nl, 
	write('Cross-reference listing written to '),
	write(File), nl, !,
	outglobals.				%  Output import/export lists if reqd

format_out(Dbase) :-
	recorded($width,width(Width),Ref),	%  linewidth
	erase(Ref),
	nl(2),
	tab(2), write('PREDICATE'), tab(15), write('FILE'),
	tab(11), write('CALLED BY'), nl(3),
	f_output(Dbase,Width).

				/* formatted output of cross refs */
f_output(Dbase,Width) :-
	member(e(F,N,f(I,Cs)),Dbase),
	f_front(F,N,I,Width), 
	f_back(Cs,38,_,Width), 
	fail.
f_output(_,_).

				/* Write predicate and file where defined */
f_front(F,N,I,W) :- 
	nl,
	f_writepred(F,N,0,C1,W),	% write predicate
	pos(C1,24),
	strippath(I,File),		% don't print path (only file).
	f_write(File,24,C2,W),		% write file
	pos(C2,38), !.

				/* Either write callers or 'NOT CALLED' */
f_back([],C1,C2,W) :- 
	f_write('NOT CALLED',C1,C2,W), !.
f_back([H|T],C1,C2,W) :- 
	write_callers([H|T],C1,C2,W), !.

				/* List callers of the predicate */
write_callers([],C,C,_) :- !.
write_callers([c(F,N)|Zs],C1,C4,W) :- 
	f_writepred(F,N,C1,C2,W),
	tab(1), C3 is C2+1,
	write_callers(Zs,C3,C4,W).

				/* Multiple newline */
nl(0) :- !.
nl(N) :- nl, N1 is N-1, nl(N1).

				/* L is the length in chars of X */
lngth(X,L) :- 
	atomic(X), !,			% Is X an atom?
	name(X,Chars),
	length(Chars,L).
lngth(/(F,N),L) :- !,			% Is it of the form F/N?
	lngth(F,L1),
	lngth(N,L2),
	L is L1+L2+1.
lngth(X,L) :-				% Must be a list
	length(X,L).
   
				/* Write predicate remembering format info */
f_writepred('<user>',0,C1,C2,W) :- !,
	makeroom(6,C1,C2,W),
	write('<user>').
f_writepred(F,N,C1,C2,W) :-
	lngth(/(F,N),L), 
	makeroom(L,C1,C2,W),
	write(F),write(/),write(N).

				/* Write atom ditto */
f_write(X,C1,C2,W) :-
	lngth(X,L), 
	makeroom(L,C1,C2,W),
	write(X).

				/* Make sure there is room to write L */ 
makeroom(L,C1,C2,W) :- 
	(L+C1 < W ; C1 =< 38), !,
	C2 is C1+L
	;  nl,
  	   tab(38),
	   C2 is 38+L.

				/* Write X in the centre of current line */
write_centre(X,W) :-
	lngth(X,L),
	tab((W-L)//2),
	writes(X).

				/* Move from column From to To */
pos(From,To) :-
	To > From, !,
	tab(To-From) ;
	nl, tab(To).

				/* Write out list of chars (string), or atom */
writes([]) :- !.
writes([C|Cs]) :- put(C), writes(Cs).
writes(X) :- write(X).

				/* Produce lists of imports and exports if
				   wanted. */
outglobals :-
	get_globals_file(File,Chars),	% Import/exp list reqd for each file?
	tell_chek(File), !,		% Yes, open file (or backtrack)
	decls_to_one_file,		% Output globals for all files
	told,
	nl, 
	write('Globals listing written to '),
	write(File), nl.

				/* Output imp/exp lists for all files */
decls_to_one_file :-
	recorded('$file'(_),'$file'(F),P),	% Yes, find a file in database
	erase(P), 				% Erase it
	write('FILE  '), write(F), nl, nl,
	outputdclrs(F), nl, nl,			% Backtrack thro all files
	fail.
decls_to_one_file.

				/* Output 'export' and 'import' declarations
				   for a file. Declarations start with
				   '%declarations%' and end with '%end%' */
outputdclrs(F) :-
	getexts(F,Lx),				%  List its imports
	getentries(F,Le),			%  List its exports
	qsort(Lx,Lsx,[]),			%  Sort import list
	qsort(Le,Lse,[]),			%  Sort export list
	write('%declarations%'), nl, nl,
	do_exports(Lse),
	do_imports(Lsx), 
	write('%end%'), nl.

				/* Build list of externals */
getexts(F,[e(G,N,f(I,_))|Exts]) :-
	recorded(F,'$ext'(F,P),Ptr),
	erase(Ptr), !,
	functor(P,G,N),
	defn_file(P,I),
	getexts(F,Exts).
getexts(_,[]).

				/* Build list of entries */
getentries(F,[e(G,N,f(File,Pr,Ar))|Ents]) :-
	recorded(F,'$entry'(F,P,c(Pr,Ar)),Ptr),
	erase(Ptr), !,
	functor(Gpr,Pr,Ar),
	defn_file(Gpr,File),
	functor(P,G,N),
	getentries(F,Ents).
getentries(_,[]).

				/* Output export decls for a file */
do_exports([]) :- !.
do_exports(L) :- 
	write('% exports:'), nl,
	put(37),tab(31),write('called by:'),tab(24-10),write('from file'),
	nl,nl,
	exports1('$nil',L), nl, !.

exports1(_,[]) :- !.
exports1(Before,[E|L]) :- one_exp_decl(Before,E), exports1(E,L).

one_exp_decl(e(F,N,_),e(F,N,f(File,Pred,Arity))) :- !,
	put(37), tab(32-1),
	writepred(Pred,Arity),
	outp_length(Pred,Arity,Length), tab(24-Length),
	strippath(File,Fn),	% only print filename
	write(Fn), nl.	

one_exp_decl(_,e(F,N,f(File,Pred,Arity))) :-
	put(37), put(9), writepred(F,N),
	outp_length(F,N,L), tab(24-L),
  	writepred(Pred,Arity),
  	outp_length(Pred,Arity,Length), tab(24-Length),
	strippath(File,Fn),
	write(Fn), nl.

				/* Output import decls for a file */
do_imports([]) :- !.
do_imports(L) :-
	write('% imports:'), nl,
	imports1(L), nl.

imports1([]).
imports1([E|L]) :- one_imp_decl(E), imports1(L).

one_imp_decl(e(F,N,f(I,_))) :-
  	put(37), put(9), writepred(F,N),
	outp_length(F,N,L), tab(24-L),
	strippath(I,Fn),
  	write('(from '), write(Fn), put(41), nl.

number_length(N,1) :- N<10, !.
number_length(N,2) :- N<100, !.
number_length(N,3).

outp_length(Pred,Arity,Length) :-
	name(Pred,L), length(L,Na),
	number_length(Arity,Nb),
	Length is Na+Nb+1.

