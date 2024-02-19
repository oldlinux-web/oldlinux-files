/*----------------------------------------------------------------------*/
/*		Directory access routine for LHarc UNIX			*/
/*		This is part of LHarc UNIX Archiver Driver		*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*	Emulate opendir(),readdir(),closedir() function for LHarc	*/
/*									*/
/*  V0.00  Original				1988.05.31  Y.Tagawa	*/
/*  V0.03  Release #3 for LHarc UNIX		1988.07.02  Y.Tagawa	*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*----------------------------------------------------------------------*/


/* DIRBLKSIZ must be sizeof (SYSTEM struct direct) * N   !!! */

#ifndef DIRBLKSIZ
#define DIRBLKSIZ	512
#endif

struct direct {
  int	d_ino;
  int	d_namlen;
  char	d_name[256];
};

typedef struct {
  int	dd_fd;
  int	dd_loc;
  int	dd_size;
  char	dd_buf[DIRBLKSIZ];
} DIR;


extern DIR *opendir ();
extern struct direct *readdir ();
extern closedir ();

