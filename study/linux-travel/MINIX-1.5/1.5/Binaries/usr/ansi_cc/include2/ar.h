/* The <ar.h> header gives the layout of archives. */

#ifndef _AR_H
#define _AR_H

#define	ARMAG	0177545
#define _NAME_MAX    14

struct ar_hdr {
  char  ar_name[_NAME_MAX];
  char  ar_date[4];		/* long in byte order 2 3 1 0 */
  char  ar_uid;
  char  ar_gid;
  char  ar_mode[2];		/* short in byte order 0 1 */
  char  ar_size[4];		/* long in byte order 2 3 1 0 */
};

#endif /* _AR_H */
