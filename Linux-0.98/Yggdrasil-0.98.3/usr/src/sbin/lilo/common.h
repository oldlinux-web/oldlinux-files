/* common.h  -  Common data structures and functions. */

/* Written 1992 by Werner Almesberger */


#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef NEW_INCLUDES
#include <linux/genhd.h>
#else
#include <linux/hdreg.h>
#endif
#include "lilo.h"


#define O_NOACCESS 3  /* open a file for "no access" */


typedef struct {
    unsigned char sector,track; /* CX */
    unsigned char device,head; /* DX */
    unsigned char num_sect; /* AL */
} SECTOR_ADDR;

typedef struct {
    char name[MAX_IMAGE_NAME+1];
    SECTOR_ADDR start;
    unsigned short swap_dev,root_dev; /* copied from image */
    unsigned short bss_seg,bss_segs,bss_words;
	/* BSS := bss_seg:0 to (bss_seg+bss_segs-1):(bss_words*2)
	   bss_seg == 0: no initialization */
} IMAGE_DESCR;

typedef struct {
    char jump[2]; /* jump over the data */
    char signature[4]; /* "LILO" */
    unsigned short stage,version;
    unsigned short delay,filler; /* delay: wait that many 54 msec units */
    SECTOR_ADDR descr;
    SECTOR_ADDR secondary[MAX_SECONDARY+1];
} BOOT_PARAMS_1; /* first stage boot loader */

typedef struct {
    char jump[2]; /* jump over the data */
    char signature[4]; /* "LILO" */
    unsigned short stage,version;
} BOOT_PARAMS_2; /* second stage boot loader */

typedef struct {
    char dummy[506];
    unsigned short swap_dev,root_dev;
} BOOT_PARAMS_L; /* root/swap information */

typedef struct {
    char jump[2]; /* jump over the data */
    char signature[4]; /* "LILO" */
    unsigned short stage,version; /* stage is 0x10 */
    unsigned short offset; /* partition entry offset */
} BOOT_PARAMS_C; /* chain loader */

#define PART(s,n) (((struct partition *) ((char *) &(s)+PART_TABLE_OFFSET)) \
  [(n)])

typedef union {
    BOOT_PARAMS_1 par_1;
    BOOT_PARAMS_2 par_2;
    BOOT_PARAMS_L par_l;
    BOOT_PARAMS_C par_c;
    unsigned char sector[SECTOR_SIZE];
} BOOT_SECTOR;

typedef union {
    IMAGE_DESCR descr[MAX_IMAGES]; /* boot file descriptors */
    unsigned char sector[SECTOR_SIZE];
} DESCR_SECTOR;


extern int verbose,compact;


void pdie(char *msg);

/* Do a perror and then exit. */

void die(char *fmt,...);

/* fprintf an error message and then exit. */

void *alloc(int size);

/* Allocates the specified number of bytes. */

char *stralloc(char *str);

/* Like strdup, but dies on error. */

int to_number(char *num);

/* Converts a string to a number. Dies if the number is invalid. */

void check_version(BOOT_SECTOR *sect,int stage);

/* Verify that a boot sector has the correct version number. */

#endif
