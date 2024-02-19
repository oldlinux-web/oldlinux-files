/*---------------------------------------------------------------------------+
 |  fpu_arith.c                                                              |
 |                                                                           |
 | Code to implement the FPU register/register arithmetis instructions       |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#include "fpu_system.h"
#include "fpu_emu.h"


void fadd__()
{
  /* fadd st,st(i) */
  reg_add(st0_ptr, &st(FPU_rm), st0_ptr);
}


void fmul__()
{
  /* fmul st,st(i) */
  reg_mul(st0_ptr, &st(FPU_rm), st0_ptr);
}



void fsub__()
{
  /* fsub st,st(i) */
  reg_sub(st0_ptr, &st(FPU_rm), st0_ptr);
}


void fsubr_()
{
  /* fsubr st,st(i) */
  reg_sub(&st(FPU_rm), st0_ptr, st0_ptr);
}


void fdiv__()
{
  /* fdiv st,st(i) */
  reg_div(st0_ptr, &st(FPU_rm), st0_ptr);
}


void fdivr_()
{
  /* fdivr st,st(i) */
  reg_div(&st(FPU_rm), st0_ptr, st0_ptr);
}



void fadd_i()
{
  /* fadd st(i),st */
  reg_add(st0_ptr, &st(FPU_rm), &st(FPU_rm));
}


void fmul_i()
{
  /* fmul st(i),st */
  reg_mul(&st(FPU_rm), st0_ptr, &st(FPU_rm));
}


void fsubri()
{
  /* fsubr st(i),st */
  /* This is the sense of the 80486 manual
     reg_sub(&st(FPU_rm), st0_ptr, &st(FPU_rm)); */
  reg_sub(st0_ptr, &st(FPU_rm), &st(FPU_rm));
}


void fsub_i()
{
  /* fsub st(i),st */
  /* This is the sense of the 80486 manual
     reg_sub(st0_ptr, &st(FPU_rm), &st(FPU_rm)); */
  reg_sub(&st(FPU_rm), st0_ptr, &st(FPU_rm));
}


void fdivri()
{
  /* fdivr st(i),st */
  reg_div(st0_ptr, &st(FPU_rm), &st(FPU_rm));
}


void fdiv_i()
{
  /* fdiv st(i),st */
  reg_div(&st(FPU_rm), st0_ptr, &st(FPU_rm));
}



void faddp_()
{
  /* faddp st(i),st */
  reg_add(st0_ptr, &st(FPU_rm), &st(FPU_rm));
  pop();
}


void fmulp_()
{
  /* fmulp st(i),st */
  reg_mul(&st(FPU_rm), st0_ptr, &st(FPU_rm));
  pop();
}



void fsubrp()
{
  /* fsubrp st(i),st */
  /* This is the sense of the 80486 manual
     reg_sub(&st(FPU_rm), st0_ptr, &st(FPU_rm)); */
  reg_sub(st0_ptr, &st(FPU_rm), &st(FPU_rm));
  pop();
}


void fsubp_()
{
  /* fsubp st(i),st */
  /* This is the sense of the 80486 manual
     reg_sub(st0_ptr, &st(FPU_rm), &st(FPU_rm)); */
  reg_sub(&st(FPU_rm), st0_ptr, &st(FPU_rm));
  pop();
}


void fdivrp()
{
  /* fdivrp st(i),st */
  reg_div(st0_ptr, &st(FPU_rm), &st(FPU_rm));
  pop();
}


void fdivp_()
{
  /* fdivp st(i),st */
  reg_div(&st(FPU_rm), st0_ptr, &st(FPU_rm));
  pop();
}

