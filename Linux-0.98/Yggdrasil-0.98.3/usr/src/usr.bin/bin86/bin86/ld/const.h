/*
 *   bin86/ld/const.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

/* const.h - constants for linker */

#define FALSE 0
#ifndef NULL
#define NULL 0
#endif
#define TRUE 1

#define EXTERN extern
#define FORWARD static
#define PRIVATE static
#define PUBLIC

#include "config.h"
