/* @(#) lzconst.h 2.1 87/12/25 12:22:30 */

/*
The contents of this file are hereby released to the public domain.

                                    -- Rahul Dhesi  1986/12/31
*/

#define  INBUFSIZ    (IN_BUF_SIZE - 10)   /* avoid obo errors */
#define  OUTBUFSIZ   (OUT_BUF_SIZE - 10)
#define  MEMERR      2
#define  IOERR       1
#define  MAXBITS     13
#define  CLEAR       256         /* clear code */
#define  Z_EOF       257         /* end of file marker */
#define  FIRST_FREE  258         /* first free code */
#define  MAXMAX      8192        /* max code + 1 */
