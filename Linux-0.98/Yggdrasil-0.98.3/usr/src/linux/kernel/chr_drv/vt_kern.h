#ifndef _VT_KERN_H
#define _VT_KERN_H

/*
 * this really is an extension of the vc_cons structure in console.c, but
 * with information needed by the vt package
 */
extern struct vt_cons {
	int		vt_mode;		/* KD_TEXT, ... */
	unsigned char	vc_kbdraw;
	unsigned char	vc_kbde0;
	unsigned char   vc_kbdleds;
} vt_cons[NR_CONSOLES];

#endif /* _VT_KERN_H */
