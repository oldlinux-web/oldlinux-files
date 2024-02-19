/* Copyright (C) 1991, 1992 Aladdin Enterprises.  All rights reserved.
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

/* zfilter2.c */
/* Additional filter creation for Ghostscript */
#include "ghost.h"
#include "errors.h"
#include "oper.h"
#include "alloc.h"
#include "dict.h"
#include "dparam.h"
#include "stream.h"

/* Imported from zfilter.c */
int filter_read(P4(os_ptr, const stream_procs _ds *, stream **, uint));
int filter_write(P4(os_ptr, const stream_procs _ds *, stream **, uint));

/* Names of keys in CCITTFax filter dictionary: */
static ref name_Uncompressed;
static ref name_K;
static ref name_EndOfLine;
static ref name_EncodedByteAlign;
static ref name_Columns;
static ref name_Rows;
static ref name_EndOfBlock;
static ref name_BlackIs1;
static ref name_DamagedRowsBeforeError;

/* Initialization */
private void
zfilter2_init()
{	static const names_def faxn[] = {

	/* Create the names of the known entries in */
	/* a CCITTFax filter dictionary. */
	   { "Uncompressed", &name_Uncompressed },
	   { "K", &name_K },
	   { "EndOfLine", &name_EndOfLine },
	   { "EncodedByteAlign", &name_EncodedByteAlign },
	   { "Columns", &name_Columns },
	   { "Rows", &name_Rows },
	   { "EndOfBlock", &name_EndOfBlock },
	   { "BlackIs1", &name_BlackIs1 },
	   { "DamagedRowsBeforeError", &name_DamagedRowsBeforeError },

	/* Mark the end of the initalized name list. */
	   names_def_end
	};

	init_names(faxn);
}

/* ------ ASCII85 filters ------ */

/* .filterASCII85Encode */
extern const stream_procs s_A85E_procs;
int
zA85E(os_ptr op)
{	return filter_write(op, &s_A85E_procs, NULL, 0);
}

/* .filterASCII85Decode */
extern const stream_procs s_A85D_procs;
int
zA85D(os_ptr op)
{	return filter_read(op, &s_A85D_procs, NULL, 0);
}

/* ------ CCITTFax filters ------ */

/* Common setup for encoding and decoding filters. */
private int
cf_setup(os_ptr op, CCITTFax_state *pcfs)
{	int code;
	check_type(*op, t_dictionary);
	check_dict_read(*op);
	if ( (code = dict_bool_param(op, &name_Uncompressed, 0,
				     &pcfs->Uncompressed)) < 0 ||
	     (code = dict_int_param(op, &name_K, -9999, 9999, 0,
				    &pcfs->K)) < 0 ||
	     (code = dict_bool_param(op, &name_EndOfLine, 0,
				     &pcfs->EndOfLine)) < 0 ||
	     (code = dict_bool_param(op, &name_EncodedByteAlign, 0,
				     &pcfs->EncodedByteAlign)) < 0 ||
	     (code = dict_int_param(op, &name_Columns, 0, 9999, 1728,
				    &pcfs->Columns)) < 0 ||
	     (code = dict_int_param(op, &name_Rows, 0, 9999, 0,
				    &pcfs->Rows)) < 0 ||
	     (code = dict_bool_param(op, &name_EndOfBlock, 1,
				     &pcfs->EndOfBlock)) < 0 ||
	     (code = dict_bool_param(op, &name_BlackIs1, 0,
				     &pcfs->BlackIs1)) < 0 ||
	     (code = dict_int_param(op, &name_DamagedRowsBeforeError, 0, 9999,
				    0, &pcfs->DamagedRowsBeforeError)) < 0
	   )
		return code;
	pcfs->raster = (pcfs->Columns + 7) >> 3;
	return 0;
}

/* .filterCCITTFaxEncode */
extern const stream_procs s_CFE_procs;
extern void s_CFE_init(P2(stream *, CCITTFax_state *));
int
zCFE(os_ptr op)
{	CCITTFax_state cfs;
	stream *s;
	int code = cf_setup(op, &cfs);
	if ( code < 0 ) return code;
	/* We need room for 2 full scan lines + 1 byte to handle 2-D. */
	code = filter_write(op - 1, &s_CFE_procs, &s, cfs.raster * 2 + 1);
	if ( code < 0 ) return code;
	s_CFE_init(s, &cfs);
	pop(1);
	return 0;
}

/* .filterCCITTFaxDecode */
extern const stream_procs s_CFD_procs;
extern void s_CFD_init(P2(stream *, CCITTFax_state *));
int
zCFD(os_ptr op)
{	CCITTFax_state cfs;
	stream *s;
	int code = cf_setup(op, &cfs);
	if ( code < 0 ) return code;
	/* We need room for 3 full scan lines to handle 2-D. */
	code = filter_read(op - 1, &s_CFD_procs, &s, cfs.raster * 3 + 1);
	if ( code < 0 ) return code;
	s_CFD_init(s, &cfs);
	pop(1);
	return 0;
}

/* ------ LZW filters ------ */

/* .filterLZWEncode */
extern const stream_procs s_LZWE_procs;
extern const uint s_LZWE_table_sizeof;
typedef struct lzw_encode_table_s lzw_encode_table;
extern void s_LZWE_init(P3(stream *, lzw_encode_table *, int));
int
zLZWE_open(os_ptr op, int enhanced)
{	stream *s;
	int code = filter_write(op, &s_LZWE_procs, &s, 0);
	lzw_encode_table *table;
	if ( code < 0 ) return code;
	table = (lzw_encode_table *)alloc(1, s_LZWE_table_sizeof,
					  "filterLZWEncode(table)");
	if ( table == 0 ) return e_VMerror;
	s_LZWE_init(s, table, enhanced);
	return code;
}
int
zLZWE(os_ptr op)
{	return zLZWE_open(op, 0);
}
int
zALZWE(os_ptr op)
{	return zLZWE_open(op, 1);
}

/* .filterLZWDecode */
extern const stream_procs s_LZWD_procs;
extern const uint s_LZWD_table_sizeof;
typedef struct lzw_decode_table_s lzw_decode_table;
extern void s_LZWD_init(P3(stream *, lzw_decode_table *, int));
int
zLZWD_open(os_ptr op, int enhanced)
{	stream *s;
	int code = filter_read(op, &s_LZWD_procs, &s, 0);
	lzw_decode_table *table;
	if ( code < 0 ) return code;
	table = (lzw_decode_table *)alloc(1, s_LZWD_table_sizeof,
					  "filterLZWDecode(table)");
	if ( table == 0 ) return e_VMerror;
	s_LZWD_init(s, table, enhanced);
	return 0;
}
int
zLZWD(os_ptr op)
{	return zLZWD_open(op, 0);
}
int
zALZWD(os_ptr op)
{	return zLZWD_open(op, 1);
}

/* ------ RunLength filters ------ */

/* .filterRunLengthEncode */
extern const stream_procs s_RLE_procs;
extern void s_RLE_init(P2(stream *, uint));
int
zRLE(register os_ptr op)
{	stream *s;
	int code;
	check_type(*op, t_integer);
	if ( (ulong)(op->value.intval) > max_uint )
		return e_rangecheck;
	code = filter_write(op - 1, &s_RLE_procs, &s, 0);
	if ( code < 0 ) return code;
	s_RLE_init(s, (uint)(op->value.intval));
	pop(1);
	return 0;
}

/* .filterRunLengthDecode */
extern const stream_procs s_RLD_procs;
extern void s_RLD_init(P1(stream *));
int
zRLD(os_ptr op)
{	stream *s;
	int code = filter_read(op, &s_RLD_procs, &s, 0);
	if ( code < 0 ) return code;
	s_RLD_init(s);
	return 0;
}

/* ------ Initialization procedure ------ */

op_def zfilter2_op_defs[] = {
	{"1.filterASCII85Encode", zA85E},
	{"1.filterASCII85Decode", zA85D},
	{"2.filterCCITTFaxEncode", zCFE},
	{"2.filterCCITTFaxDecode", zCFD},
	{"1.filterLZWDecode", zLZWD},
	{"1.filterLZWEncode", zLZWE},
	{"2.filterRunLengthEncode", zRLE},
	{"1.filterRunLengthDecode", zRLD},
	op_def_end(zfilter2_init)
};
