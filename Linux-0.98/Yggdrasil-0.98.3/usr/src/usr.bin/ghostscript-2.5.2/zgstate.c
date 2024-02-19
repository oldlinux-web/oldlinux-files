/* Copyright (C) 1989, 1992 Aladdin Enterprises.  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of Ghostscript.

Ghostscript is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the Ghostscript General Public License for full details.

Everyone is granted permission to copy, modify and redistribute
Ghostscript, but only under the conditions described in the Ghostscript
General Public License.  A copy of this license is supposed to have been
given to you along with Ghostscript so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.  */

/* zgstate.c */
/* Graphics state operators for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "alloc.h"
#include "gsmatrix.h"
#include "gsstate.h"
#include "state.h"
#include "store.h"

/* Forward references */
private int near num_param(P2(os_ptr, int (*)(P2(gs_state *, floatp))));
private int near line_param(P2(os_ptr, int *));

/* ------ Operations on the entire graphics state ------ */

/* The current graphics state */
gs_state *igs;
int_gstate istate;

/* "Client" procedures */
private char/*void*/ *gs_istate_alloc(P1(const gs_memory_procs *mp));
private int gs_istate_copy(P2(char/*void*/ *to, const char/*void*/ *from));
private void gs_istate_free(P2(char/*void*/ *old, const gs_memory_procs *mp));
private const gs_state_client_procs istate_procs = {
	gs_istate_alloc,
	gs_istate_copy,
	gs_istate_free
};

/* Initialize the graphics stack. */
void
gs_init()
{	igs = gs_state_alloc(alloc, alloc_free);
	make_null(&istate.screen_proc);
	make_null(&istate.transfer_procs.red);
	make_null(&istate.transfer_procs.green);
	make_null(&istate.transfer_procs.blue);
	make_null(&istate.transfer_procs.gray);
	make_null(&istate.colorspace);
	make_null(&istate.pattern);
	make_null(&istate.colorrendering);
	make_null(&istate.halftone);
	gs_state_set_client(igs, (char/*void*/ *)&istate, &istate_procs);
	/* gsave and grestore only work properly */
	/* if there are always at least 2 entries on the stack. */
	/* We count on the PostScript initialization code to do a gsave. */
}

/* gsave */
int
zgsave(register os_ptr op)
{	return gs_gsave(igs);
}

/* grestore */
int
zgrestore(register os_ptr op)
{	return gs_grestore(igs);
}

/* grestoreall */
int
zgrestoreall(register os_ptr op)
{	return gs_grestoreall(igs);
}

/* initgraphics */
int
zinitgraphics(register os_ptr op)
{	return gs_initgraphics(igs);
}

/* ------ Operations on graphics state elements ------ */

/* setlinewidth */
int
zsetlinewidth(register os_ptr op)
{	return num_param(op, gs_setlinewidth);
}

/* currentlinewidth */
int
zcurrentlinewidth(register os_ptr op)
{	push(1);
	make_real(op, gs_currentlinewidth(igs));
	return 0;
}

/* setlinecap */
int
zsetlinecap(register os_ptr op)
{	int param;
	int code = line_param(op, &param);
	if ( !code ) code = gs_setlinecap(igs, (gs_line_cap)param);
	return code;
}

/* currentlinecap */
int
zcurrentlinecap(register os_ptr op)
{	push(1);
	make_int(op, (int)gs_currentlinecap(igs));
	return 0;
}

/* setlinejoin */
int
zsetlinejoin(register os_ptr op)
{	int param;
	int code = line_param(op, &param);
	if ( !code ) code = gs_setlinejoin(igs, (gs_line_join)param);
	return code;
}

/* currentlinejoin */
int
zcurrentlinejoin(register os_ptr op)
{	push(1);
	make_int(op, (int)gs_currentlinejoin(igs));
	return 0;
}

/* setmiterlimit */
int
zsetmiterlimit(register os_ptr op)
{	return num_param(op, gs_setmiterlimit);
}

/* currentmiterlimit */
int
zcurrentmiterlimit(register os_ptr op)
{	push(1);
	make_real(op, gs_currentmiterlimit(igs));
	return 0;
}

/* setdash */
int
zsetdash(register os_ptr op)
{	float offset;
	uint n, i;
	const ref *dfrom;
	float *pattern, *dto;
	int code = real_param(op, &offset);
	if ( code ) return code;
	check_array(op[-1]);
	check_read(op[-1]);
	/* Unpack the dash pattern and check it */
	dfrom = op[-1].value.const_refs;
	i = n = r_size(op - 1);
	pattern = dto = (float *)alloc(n, sizeof(float), "setdash");
	while ( i-- )
	   {	switch ( r_type(dfrom) )
		   {
		case t_integer:
			*dto++ = dfrom->value.intval;
			break;
		case t_real:
			*dto++ = dfrom->value.realval;
			break;
		default:
			alloc_free((char *)dto, n, sizeof(float), "setdash");
			return e_typecheck;
		   }
		dfrom++;
	   }
	code = gs_setdash(igs, pattern, n, offset);
	if ( !code ) pop(2);
	return code;
}

/* currentdash */
int
zcurrentdash(register os_ptr op)
{	int n = gs_currentdash_length(igs);
	int i = n;
	ref *pattern = alloc_refs(n, "currentdash");
	ref *dto = pattern;
	float *dfrom = (float *)((char *)pattern + n * (sizeof(ref) - sizeof(float)));
	gs_currentdash_pattern(igs, dfrom);
	while ( i-- )
	   {	make_real(dto, *dfrom);
		dto++, dfrom++;
	   }
	push(2);
	make_tasv(op - 1, t_array, a_all, n, refs, pattern);
	make_real(op, gs_currentdash_offset(igs));
	return 0;
}

/* setflat */
int
zsetflat(register os_ptr op)
{	return num_param(op, gs_setflat);
}

/* currentflat */
int
zcurrentflat(register os_ptr op)
{	push(1);
	make_real(op, gs_currentflat(igs));
	return 0;
}

/* setstrokeadjust */
int
zsetstrokeadjust(register os_ptr op)
{	check_type(*op, t_boolean);
	gs_setstrokeadjust(igs, op->value.index);
	pop(1);
	return 0;
}

/* currentstrokeadjust */
int
zcurrentstrokeadjust(register os_ptr op)
{	push(1);
	make_bool(op, gs_currentstrokeadjust(igs));
	return 0;
}

/* ------ Initialization procedure ------ */

op_def zgstate_op_defs[] = {
	{"0currentdash", zcurrentdash},
	{"0currentflat", zcurrentflat},
	{"0currentlinecap", zcurrentlinecap},
	{"0currentlinejoin", zcurrentlinejoin},
	{"0currentlinewidth", zcurrentlinewidth},
	{"0currentmiterlimit", zcurrentmiterlimit},
	{"0currentstrokeadjust", zcurrentstrokeadjust},
	{"0grestore", zgrestore},
	{"0grestoreall", zgrestoreall},
	{"0gsave", zgsave},
	{"0initgraphics", zinitgraphics},
	{"2setdash", zsetdash},
	{"1setflat", zsetflat},
	{"1setlinecap", zsetlinecap},
	{"1setlinejoin", zsetlinejoin},
	{"1setlinewidth", zsetlinewidth},
	{"1setmiterlimit", zsetmiterlimit},
	{"1setstrokeadjust", zsetstrokeadjust},
	op_def_end(0)
};

/* ------ Internal routines ------ */

/* Allocate the interpreter's part of a graphics state. */
private char/*void*/ *
gs_istate_alloc(const gs_memory_procs *mp)
{	return (char/*void*/ *)alloc_refs(sizeof(int_gstate) / sizeof(ref), "int_gsave");
}

/* Copy the interpreter's part of a graphics state. */
private int
gs_istate_copy(char/*void*/ *to, const char/*void*/ *from)
{	*(int_gstate *)to = *(int_gstate *)from;
	return 0;
}

/* Free the interpreter's part of a graphics state. */
private void
gs_istate_free(char/*void*/ *old, const gs_memory_procs *mp)
{	alloc_free_refs((ref *)old, sizeof(int_gstate) / sizeof(ref), "int_grestore");
}

/* Get a numeric parameter */
private int near
num_param(os_ptr op, int (*pproc)(P2(gs_state *, floatp)))
{	float param;
	int code = real_param(op, &param);
	if ( !code ) code = (*pproc)(igs, param);
	if ( !code ) pop(1);
	return code;
}

/* Get an integer parameter 0-2. */
private int near
line_param(register os_ptr op, int *pparam)
{	check_type(*op, t_integer);
	if ( op->value.intval < 0 || op->value.intval > 2 )
		return e_rangecheck;
	*pparam = (int)op->value.intval;
	pop(1);
	return 0;
}
