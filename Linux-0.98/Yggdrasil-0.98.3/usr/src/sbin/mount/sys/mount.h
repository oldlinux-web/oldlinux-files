/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)mount.h	7.22 (Berkeley) 6/3/91
 */

/* For Linux I removed most of the stuff that we don't support, although
   I left some things that we might use soon.  The big change will be NFS.  */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

/* We've got this in <sys/vfs.h> right now.  Should it be moved here?  */
#include <sys/vfs.h>
#define MNAMELEN 90	/* length of buffer for returned name */

#if 0
struct statfs {
	short	f_type;			/* type of filesystem (see below) */
	short	f_flags;		/* copy of mount flags */
	long	f_fsize;		/* fundamental file system block size */
	long	f_bsize;		/* optimal transfer block size */
	long	f_blocks;		/* total data blocks in file system */
	long	f_bfree;		/* free blocks in fs */
	long	f_bavail;		/* free blocks avail to non-superuser */
	long	f_files;		/* total file nodes in file system */
	long	f_ffree;		/* free file nodes in fs */
	fsid_t	f_fsid;			/* file system id */
	long	f_spare[9];		/* spare for later */
	char	f_mntonname[MNAMELEN];	/* directory on which mounted */
	char	f_mntfromname[MNAMELEN];/* mounted filesystem */
};

#endif /* 0 */

/*
 * File system types for BSD statfs(2) and mount(2). 
 * Linux statfs(2) should return these, but Linux mount(2) uses name
 * strings themselves rather than integer values.  Of course we need
 * a value for the Minix and ext FS here.
 */
#define	MOUNT_NONE	0
#define	MOUNT_UFS	1
#define	MOUNT_NFS	2
#define	MOUNT_MFS	3
#define	MOUNT_PC	4
#define	MOUNT_MAXTYPE	4


/*
 * Mount flags.  These go in the mask (last argument to mount(2)).
 * The Linux values are different from the BSD values.  This is
 * unfortunate, but we can live with it.
 */
#define MNT_DEFAULT	0	/* default mount options */
#define	MNT_RDONLY	1	/* read only filesystem */
#define	MNT_NOEXEC	8	/* can't exec from filesystem */
#define	MNT_NOSUID	2	/* don't honor setuid bits on fs */
#define	MNT_NODEV	4	/* don't interpret special files */

#if 0
#define MNT_DEFAULT	0x00000000	/* default mount options */
#define	MNT_RDONLY	0x00000001	/* read only filesystem */
#define	MNT_SYNCHRONOUS	0x00000002	/* file system written synchronously */
#define	MNT_NOEXEC	0x00000004	/* can't exec from filesystem */
#define	MNT_NOSUID	0x00000008	/* don't honor setuid bits on fs */
#define	MNT_NODEV	0x00000010	/* don't interpret special files */

/*
 * Mask of flags that are visible to statfs()
 */
#define	MNT_VISFLAGMASK	0x0000ffff
#endif /* 0 */

int fstatfs (int __fd, struct statfs *__buf);
int statfs (const char *__path, struct statfs *__buf);

/* 4.3BSD Net-2 mount(2)/unmount(2) are a bit different from Linux'.
 * Note that BSD umount has been enhanced and renamed unmount.
 *	int	mount (int, const char *, int, void *);
 *	int	unmount (const char *, int);
 */

#ifdef HAVE_MOUNT5
/* 0.96c-pl1 and later we have a five argument mount(2).  */
int mount (const char *__special, const char *__dir,
	   const char *__type, unsigned long __flags, const void *__data);
#else
/* Before 0.96c-pl1 we had a four argument mount(2).  */
int mount (const char *__special, const char *__dir,
	   const char *__type, unsigned long __flags);
#endif

int umount (const char *__special);

int swapon (const char *__special);
int swapoff (const char *__special);

#endif /* _SYS_MOUNT_H_ */
