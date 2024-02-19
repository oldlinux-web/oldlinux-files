/* errors.c */
extern void Un_impl(void);
extern void emu_printall(void);
extern void exception(int n);
extern void real_2op_NaN(struct reg *a, struct reg *b, struct reg *dest);
extern void arith_invalid(struct reg *dest);
extern void divide_by_zero(int sign, struct reg *dest);
extern void arith_overflow(struct reg *dest);
extern void arith_underflow(struct reg *dest);
extern void stack_overflow(void);
extern void stack_underflow(void);
/* fpu_arith.c */
extern void fadd__(void);
extern void fmul__(void);
extern void fsub__(void);
extern void fsubr_(void);
extern void fdiv__(void);
extern void fdivr_(void);
extern void fadd_i(void);
extern void fmul_i(void);
extern void fsubri(void);
extern void fsub_i(void);
extern void fdivri(void);
extern void fdiv_i(void);
extern void faddp_(void);
extern void fmulp_(void);
extern void fsubrp(void);
extern void fsubp_(void);
extern void fdivrp(void);
extern void fdivp_(void);
/* fpu_aux.c */
extern void finit(void);
extern void finit_(void);
extern void fstsw_(void);
extern void fp_nop(void);
extern void fld_i_(void);
extern void fxch_i(void);
extern void ffree_(void);
extern void fst_i_(void);
extern void fstp_i(void);
/* fpu_entry.c */
extern void math_emulate(long arg);
/* fpu_etc.c */
extern void fp_etc(void);
/* fpu_trig.c */
extern void convert_l2reg(long *arg, struct reg *dest);
extern void trig_a(void);
extern void trig_b(void);
/* get_address.c */
extern void get_address(void);
/* load_store.c */
extern void load_store_instr(char type);
/* poly_2xm1.c */
extern int poly_2xm1(struct reg *arg, struct reg *result);
/* poly_atan.c */
extern void poly_atan(struct reg *arg);
extern void poly_add_1(struct reg *src);
/* poly_l2.c */
extern void poly_l2(struct reg *arg, struct reg *result);
extern int poly_l2p1(struct reg *arg, struct reg *result);
/* poly_sin.c */
extern void poly_sine(struct reg *arg, struct reg *result);
/* poly_tan.c */
extern void poly_tan(struct reg *arg, struct reg *y_reg);
/* reg_add_sub.c */
extern void reg_add(struct reg *a, struct reg *b, struct reg *dest);
extern void reg_sub(struct reg *a, struct reg *b, struct reg *dest);
/* reg_compare.c */
extern int compare(struct reg *b);
extern void compare_st_data(void);
extern void fcom_st(void);
extern void fcompst(void);
extern void fcompp(void);
extern void fucom_(void);
extern void fucomp(void);
extern void fucompp(void);
/* reg_constant.c */
extern void fconst(void);
/* reg_ld_str.c */
extern void reg_load_extended(void);
extern void reg_load_double(void);
extern void reg_load_single(void);
extern void reg_load_int64(void);
extern void reg_load_int32(void);
extern void reg_load_int16(void);
extern void reg_load_bcd(void);
extern int reg_store_extended(void);
extern int reg_store_double(void);
extern int reg_store_single(void);
extern int reg_store_int64(void);
extern int reg_store_int32(void);
extern int reg_store_int16(void);
extern int reg_store_bcd(void);
extern int round_to_int(struct reg *r);
extern char *fldenv(void);
extern void frstor(void);
extern char *fstenv(void);
extern void fsave(void);
/* reg_mul.c */
extern void reg_mul(struct reg *a, struct reg *b, struct reg *dest);
