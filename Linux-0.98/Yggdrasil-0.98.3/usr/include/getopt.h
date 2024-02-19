#ifndef _GETOPT_H_
#define _GETOPT_H_

#include <traditional.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *optarg;
extern int optind;
extern int opterr;

struct option
{
#ifdef __STDC__
  const
#endif
  char *name;
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */

enum _argtype
{
  no_argument,
  required_argument,
  optional_argument
};

extern int getopt _ARGS ((int __argc, char *const *__argv, 
			const char *__shortopts));
extern int getopt_long _ARGS ((int __argc, char *const *__argv,
			const char *__shortopts,
		        const struct option *__longopts, int *__longind));
extern int getopt_long_only _ARGS ((int __argc, char *const *__argv,
			     const char *__shortopts,
		             const struct option *__longopts,
			     int *__longind));

/* Internal only.  Users should not call this directly.  */
extern int _getopt_internal _ARGS ((int __argc, char *const *__argv,
			     const char *__shortopts,
		             const struct option *__longopts,
			     int *__longind, int __long_only));
#ifdef __cplusplus
}
#endif

#endif /* _GETOPT_H_ */
