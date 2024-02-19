/*
 *   bin86/as/error.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* error.c - error routines for assembler */

#include <string.h>
#include "const.h"
#include "type.h"

PRIVATE char *errormessage[] =
{
    "comma expected",
    "delimiter expected",
    "factor expected",
    "index register expected",
    "label expected",
    "left parentheses expected",
    "opcode expected",
    "right bracket expected",
    "register expected",
    "right parentheses expected",
    "space expected",
    "absolute expression required",
    "non-imported expression required",
    "relocation impossible",
    "illegal label",
    "MACRO used as identifier",
    "missing label",
    "opcode used as identifier",
    "register used as identifier",
    "redefined label",
    "unbound label",
    "undefined label",
    "variable used as label",
    "address out of bounds",
    "data out of bounds",
    "illegal address mode",
    "illegal register",
    "no matching IF",
    "no matching BLOCK",
    "end of file in BLOCK",
    "end of file in IF",
    "location counter was undefined at end",
    "end of file in MACRO",
    "user-generated error",
    "BLOCK stack overflow",
    "binary file wrap-around",
    "counter overflow",
    "counter underflow",
    "GET stack overflow",
    "IF stack overflow",
    "line too long",
    "MACRO stack overflow",
    "object symbol table overflow",
    "program overwrite",
    "parameter table overflow",
    "symbol table overflow",
    "output symbol table overflow",
    "error writing object file",
    "al, ax or eax expected",
    "control character in string",
    "futher errors suppressed",
    "illegal immediate mode",
    "illegal indirect to indirect",
    "illegal indirection",
    "illegal indirection from previous 'ptr'",
    "illegal scale",
    "illegal section",
    "illegal segment register",
    "illegal source effective address",
    "illegal size",
    "immediate expression expected",
    "index register expected",
    "indirect expression required",
    "mismatched size",
    "no imports with binary file output",
    "multiple ENTER pseudo-ops",
    "relative expression required",
    "repeated displacement",
    "segment or relocatability redefined",
    "segment register required",
    "size unknown",
    "FP register required",
    "FP register not allowed",
    "illegal FP register",
    "illegal FP register pair",
    "junk after operands",
    "already defined",
    "short branch would do",
    "unknown error",
};

/* build null-terminated error message for given error at given spot */

PUBLIC char *build_error_message(errnum, buf)
unsigned errnum;
char *buf;
{
    if (errnum >= sizeof errormessage / sizeof errormessage[0])
	errnum = sizeof errormessage / sizeof errormessage[0] - 1;
    return strcpy(buf, errormessage[errnum]);
}
