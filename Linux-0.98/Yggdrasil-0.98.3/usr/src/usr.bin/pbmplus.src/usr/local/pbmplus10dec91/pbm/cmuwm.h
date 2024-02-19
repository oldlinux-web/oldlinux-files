/* cmuwm.h - definitions for the CMU window manager format
*/

#ifndef _CMUWM_H_
#define _CMUWM_H_

struct cmuwm_header
    {
    long magic;
    long width;
    long height;
    short depth;
    };

#define CMUWM_MAGIC 0xf10040bbL

#endif /*_CMUWM_H_*/
