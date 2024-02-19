/*
 *   bin86/bccfp/fplib.h
 *
 *   Copyright (C) 1992 Bruce Evans
 */

#define ALIGNMENT	4
#define CHAR_BIT	8
#define D_BIT		(D_SIZE * CHAR_BIT)
#define D_EXP_BIAS	((1 << (D_EXP_BIT - 1)) - 1)
#define D_EXP_BIT	11
#define D_EXP_INFINITE	((1 << D_EXP_BIT) - 1)
#define D_EXP_MASK	(((1 << D_EXP_BIT) - 1) << D_EXP_SHIFT)
#define D_EXP_SHIFT	(REG_BIT - (1 + D_EXP_BIT))
#define D_FRAC_BIT	53
#define D_FRAC_MASK	(D_NORM_MASK - 1)
#define D_HIGH		4
#define D_HUGE_HIGH	(D_EXP_MASK - 1)
#define D_HUGE_LOW	0xFFFFFFFF
#define D_LOW		0
#define D_NORM_BIT	(D_FRAC_BIT - 1 - REG_BIT)
#define D_NORM_MASK	(1 << D_NORM_BIT)
#define D_SIGN_BIT	63
#define D_SIGN_MASK	(1 << (D_SIGN_BIT - REG_BIT))
#define D_SIZE		8
#define F_BIT		(F_SIZE * CHAR_BIT)
#define F_EXP_BIAS	((1 << (F_EXP_BIT - 1)) - 1)
#define F_EXP_BIT	8
#define F_EXP_INFINITE	((1 << F_EXP_BIT) - 1)
#define F_EXP_MASK	(((1 << F_EXP_BIT) - 1) << F_EXP_SHIFT)
#define F_EXP_SHIFT	(REG_BIT - (1 + F_EXP_BIT))
#define F_FRAC_BIT	24
#define F_FRAC_MASK	(F_NORM_MASK - 1)
#define F_HIGH		0
#define F_HUGE_HIGH	(F_EXP_MASK - 1)
#define F_NORM_BIT	(F_FRAC_BIT - 1)
#define F_NORM_MASK	(1 << F_NORM_BIT)
#define F_SIGN_BIT	31
#define F_SIGN_MASK	(1 << F_SIGN_BIT)
#define F_SIZE		4
#define FREE_D_SIGN_BIT_TEST	(D_SIGN_BIT % REG_BIT == REG_BIT - 1)
#define GENREG_SIZE	4
#define INT_BIT		32
#define INT_MAX		0x7FFFFFFF
#define INT_MIN		(-0x7FFFFFFF - 1)
#define PC_SIZE		4
#define REG_BIT		32
#define SHORT_BIT	16
#define UINT_MAX	0xFFFFFFFF
