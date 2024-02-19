/* variables.h -- data structures for shell variables. */

/* What a shell variable looks like. */
typedef struct variable {
  struct variable *next;	/* Next variable in the list. */
  char *name;			/* Symbol that the user types. */
  char *value;			/* Value that is returned. */
  char *function;		/* Function cell. */
  int attributes;		/* export, readonly, array, invisible... */
  int context;			/* Which context this variable belongs to. */
  struct variable *prev_context; /* Value from previous context or NULL. */
} SHELL_VAR;

/* The various attributes that a given variable can have.
   We only reserve one byte of the INT. */
#define att_exported  0x01	/* %00000001 (export to environment) */
#define att_readonly  0x02	/* %00000010 (cannot change)	     */
#define att_invisible 0x04	/* %00000100 (cannot see)	     */
#define att_array     0x08	/* %00001000 (value is an array)     */
#define att_nounset   0x10	/* %00010000 (cannot unset)	     */

/* This is on the way out.  It is only here so that
   set_or_show_variables () has a flag value for the "-f" flag. */
#define att_function  0x20	/* %00100000 */

#define exported_p(var)		((((var)->attributes) & (att_exported)))
#define readonly_p(var)		((((var)->attributes) & (att_readonly)))
#define invisible_p(var)	((((var)->attributes) & (att_invisible)))
#define array_p(var)		((((var)->attributes) & (att_array)))
#define function_p(var)		((((var)->function) != (char *)NULL))

#define function_cell(var) ((var)->function)
#define value_cell(var) ((var)->value)

/* Stuff for hacking variables. */
extern SHELL_VAR *variable_list, *bind_variable (), *find_variable ();
extern SHELL_VAR *copy_variable ();
extern char *get_string_value (), *dollar_vars[];
extern char **export_env;

extern int variable_context;
