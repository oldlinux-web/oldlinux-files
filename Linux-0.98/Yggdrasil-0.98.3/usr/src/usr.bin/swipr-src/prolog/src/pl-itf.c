/*  @(#) pl-itf.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: foreign language interface
*/

#include "pl-incl.h"
#include "pl-itf.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This modules  defines  the  functions  available  to  users  of  foreign
language  code.   Most  of  this  module  just is a small function layer
around primitives, normally provided via macros.   This  module  is  not
responsible for loading foreign language code (see pl-load.c). Note that
on  systems  on which pl-load.c is not portable, one can still use these
functions, link the .o files while linking prolog and call  the  foreign
module's initialisation function from main() in pl-main.c.  PCE normally
is linked this way.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

		/********************************
		*           ANALYSIS            *
		*********************************/

int
PL_type(t)
register Word t;
{ if ( isVar(*t) )		return PL_VARIABLE;
  if ( isInteger(*t) )		return PL_INTEGER;
  if ( isReal(*t) )		return PL_FLOAT;
#if O_STRING
  if ( isString(*t) )		return PL_STRING;
#endif O_STRING
  if ( isAtom(*t) )		return PL_ATOM;
  if ( isTerm(*t) )		return PL_TERM;

  return sysError("PL_type(): illegal type");
}

double					/* double for standard arg passing */
PL_float_value(t)
register word t;
{ return valReal(t);
}

#if O_STRING
char *
PL_string_value(t)
register word t;
{ return valString(t);
}
#endif O_STRING

long
PL_integer_value(t)
register word t;
{ return valNum(t);
}

char *
PL_atom_value(t)
register word t;
{ return stringAtom(t);
}

functor
PL_functor(t)
register Word t;
{ return (functor) functorTerm(*t);
}

atomic
PL_functor_name(f)
register FunctorDef f;
{ return (atomic) f->name;
}

int
PL_functor_arity(f)
register FunctorDef f;
{ return f->arity;
}

term
PL_arg(t, n)
register Word t;
register int n;
{ register Word a = argTermP(*t, n-1);

  deRef(a);

  return (term) a;
}

term
PL_strip_module(t, m)
term t;
Module *m;
{ return (term) stripModule(t, m);
}

		/********************************
		*         CONSTRUCTION          *
		*********************************/

term
PL_new_term()
{ register Word var = allocGlobal(sizeof(word));

  setVar(*var);
  return var;
}

atomic
PL_new_atom(s)
char *s;
{ return (atomic) lookupAtom(s);
}

atomic
PL_new_integer(i)
int i;
{ return (atomic) consNum(i);
}

#if O_STRING
atomic
PL_new_string(s)
char *s;
{ return (atomic) globalString(s);
}
#endif O_STRING

atomic
PL_new_float(f)
double f;
{ return (atomic) globalReal(f);
}

functor
PL_new_functor(f, a)
register atomic f;
register int a;
{ return (functor) lookupFunctorDef((Atom)f, a);
}

bool
PL_unify(t1, t2)
register Word t1, t2;
{ return (bool) pl_unify(t1, t2);
}

bool
PL_unify_atomic(t, w)
register Word t;
register word w;
{ return unifyAtomic(t, w);
}

bool
PL_unify_functor(t, f)
register Word t;
register FunctorDef f;
{ return unifyFunctor(t, f);
}

		/********************************
		*      REGISTERING FOREIGNS     *
		*********************************/

static bool registerForeign P((char *, int, Func, va_list));

static bool
registerForeign(name, arity, f, args)
char *name;
int arity;
Func f;
va_list args;
{ static word input;
  SourceFile sf;
  Procedure proc;
  Definition def;
  int n;
  Module m;
  int attribute;

  setVar(input);
  pl_seeing(&input);
  sf = lookupSourceFile((Atom)input);

  m = (environment_frame ? contextModule(environment_frame)
			 : MODULE_system);

  proc = lookupProcedure(lookupFunctorDef(lookupAtom(name), arity), m);
  def = proc->definition;

  if ( true(def, SYSTEM) )
    return warning("PL_register_foreign(): Attempt to redefine a system predicate: %s",
							procedureName(proc));
  if ( def->source != (SourceFile) NULL && def->source != sf )
    warning("PL_register_foreign(): redefined %s", procedureName(proc));
  def->source = sf;

  if ( false(def, FOREIGN) && def->definition.clauses != (Clause) NULL )
    abolishProcedure(proc, m);

  def->definition.function = f;
  def->indexPattern = 0;
  def->indexCardinality = 0;
  def->flags = 0;
  set(def, FOREIGN|TRACE_ME);
  clear(def, NONDETERMINISTIC);

  for(n=0; (attribute = va_arg(args, int)) != 0; n++ )
  { switch( attribute )
    { case PL_FA_NOTRACE:	   clear(def, TRACE_ME);	break;
      case PL_FA_TRANSPARENT:	   set(def, TRANSPARENT);	break;
      case PL_FA_NONDETERMINISTIC: set(def, NONDETERMINISTIC);	break;
    }
    if ( n > 3 )
      return warning("PL_register_foreign(): %s/%d: argument list not closed",
								name, arity);
  }

  succeed;
}  

#if ANSI && !AIX
bool
PL_register_foreign(char *name, int arity, Func f, ...)
{ va_list args;
  bool rval;

  va_start(args, f);
  rval = registerForeign(name, arity, f, args);
  va_end(args);

  return rval;
}

#else

bool
PL_register_foreign(va_alist)
va_dcl
{ va_list args;
  char *name;
  int arity;
  Func f;
  bool rval;

  va_start(args);
  name  = va_arg(args, char *);
  arity = va_arg(args, int);
  f     = va_arg(args, Func);
  rval = registerForeign(name, arity, f, args);
  va_end(args);

  return rval;
}
#endif

		/********************************
		*        CALLING PROLOG         *
		*********************************/

void
PL_mark(buf)
register bktrk_buf *buf;
{ Mark(*((mark *)buf));
}

void
PL_bktrk(buf)
register bktrk_buf *buf;
{ Undo(*((mark *)buf));
}

bool
PL_call(t, m)
Word t;
Module m;
{ LocalFrame lSave   = lTop;
  LocalFrame envSave = environment_frame;
  Word *     aSave   = aTop;
  bool	     rval;

  deRef(t);

  if ( m == (Module) NULL )
    m = contextModule(environment_frame);

  lTop = (LocalFrame) addPointer(lTop, sizeof(LocalFrame));
  verifyStack(local);
  varFrame(lTop, -1) = (word) environment_frame;

  gc_status.blocked++;
  rval = interpret(m, *t, TRUE);
  gc_status.blocked--;

  lTop		    = lSave;
  aTop		    = aSave;
  environment_frame = envSave;

  return rval;
}  

		/********************************
		*            MODULES            *
		*********************************/

module
PL_context()
{ return (module) contextModule(environment_frame);
}

atomic
PL_module_name(m)
register Module m;
{ return (atomic) m->name;
}

module
PL_new_module(name)
register atomic name;
{ return (module) lookupModule((Atom) name);
}

#if unix
		/********************************
		*            SIGNALS            *
		*********************************/

void
(*PL_signal(sig, func))()
int sig;
void (*func)();
{ void (*old)();

  if ( sig < 0 || sig >= MAXSIGNAL )
  { fatalError("PL_signal(): illegal signal number: %d", sig);
    return NULL;
  }

  if ( signalHandlers[sig].catched == FALSE )
  { old = signal(sig, func);
    signalHandlers[sig].os = func;
    
    return old;
  }

  old = signalHandlers[sig].user;
  signalHandlers[sig].user = func;

  return old;
}
#endif

		/********************************
		*           WARNINGS            *
		*********************************/

#if ANSI && !AIX
bool
PL_warning(char *fm, ...)
{ va_list args;

  va_start(args, fm);
  vwarning(fm, args);
  va_end(args);

  fail;
}

void
PL_fatal_error(char *fm, ...)
{ va_list args;

  va_start(args, fm);
  vfatalError(fm, args);
  va_end(args);
}

#else

bool
PL_warning(va_alist)
va_dcl
{ char *fm;
  va_list args;

  va_start(args);
  fm = va_arg(args, char *);
  vwarning(fm, args);
  va_end(args);

  fail;
}

void
PL_fatal_error(va_alist)
va_dcl
{ char *fm;
  va_list args;

  va_start(args);
  fm = va_arg(args, char *);
  vfatalError(fm, args);
  va_end(args);
}
#endif ANSI

		/********************************
		*            ACTIONS            *
		*********************************/

bool
PL_action(action, arg)
int action;
long arg;
{ switch(action)
  { case PL_ACTION_TRACE:	return (bool) pl_trace();
    case PL_ACTION_DEBUG:	return (bool) pl_debug();
    case PL_ACTION_BACKTRACE:	backTrace(environment_frame); succeed;
    case PL_ACTION_BREAK:	return (bool) pl_break();
    case PL_ACTION_HALT:	return (bool) pl_halt();
    case PL_ACTION_ABORT:	return (bool) pl_abort();
    case PL_ACTION_SYMBOLFILE:	loaderstatus.symbolfile = lookupAtom((char *) arg);
				succeed;
    default:			sysError("PL_action(): Illegal action: %d", action);
				/*NOTREACHED*/
				fail;
  }
}

		/********************************
		*         QUERY PROLOG          *
		*********************************/

long
PL_query(query)
int query;
{ switch(query)
  { case PL_QUERY_ARGC:		 return (long) mainArgc;
    case PL_QUERY_ARGV:		 return (long) mainArgv;
    case PL_QUERY_SYMBOLFILE:	 if ( getSymbols() == FALSE )
				   return (long) NULL;
				 return (long) stringAtom(loaderstatus.symbolfile);
    case PL_QUERY_ORGSYMBOLFILE: if ( getSymbols() == FALSE )
				   return (long) NULL;
				 return (long) stringAtom(loaderstatus.orgsymbolfile);
    default:			 sysError("PL_query: Illegal query: %d", query);
				 /*NOTREACHED*/
				 fail;
  }
}
