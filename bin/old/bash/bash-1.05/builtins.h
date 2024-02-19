/* builtins.h -- a list of all commands that are shell builtins.  See
   builtins.c to see where to add more builtins. */

#include "config.h"

int colon_builtin (), period_builtin (), break_builtin (),
  continue_builtin (), cd_builtin (), echo_builtin (), eval_builtin (),
  exec_builtin (), exit_builtin (), export_builtin (), hash_builtin (),
  pwd_builtin (), read_builtin (), readonly_builtin (), return_builtin (),
  set_builtin (), shift_builtin (), test_builtin (), times_builtin (),
  trap_builtin (), type_builtin (), ulimit_builtin (), umask_builtin (),
  unset_builtin (), wait_builtin (), help_builtin (), enable_builtin (),
  history_builtin (), logout_builtin (), builtin_builtin (),
  command_builtin ();

int declare_builtin (), local_builtin ();

#ifdef PUSHD_AND_POPD
int dirs_builtin (), pushd_builtin (), popd_builtin ();
#endif  /* PUSHD_AND_POPD */

#ifdef ALIAS
#include "alias.h"
int alias_builtin (), unalias_builtin ();
#endif  /* ALIAS */

#ifdef JOB_CONTROL
int jobs_builtin (), fg_builtin (), bg_builtin ();
int kill_builtin (), suspend_builtin ();
#endif

/* The thing that we build the array of builtins out of. */
struct builtin {
  char *name;
  Function *function;
  int enabled;
  char *short_doc;
  char *long_doc;
};

extern struct builtin shell_builtins[];

