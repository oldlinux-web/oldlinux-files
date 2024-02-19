/* pjtoppm.c - convert an HP PainJetXL image to a portable pixmap file
**
** Copyright (C) 1990 by Christos Zoulas (christos@ee.cornell.edu)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

static char usage[] =  "[paintjetfile]";

static int
egetc(fp)
FILE *fp;
{
    int c;
    if ((c = fgetc(fp)) == -1)
	pm_error("unexpected end of file");
    return(c);
}

main(argc, argv)
int argc;
char *argv[];
{
    int cmd, val;
    char buffer[BUFSIZ];
    int planes = 3, rows = -1, cols = -1;
    int r = 0, c = 0, p = 0, i;
    unsigned char **image = NULL;
    int *imlen;
    FILE *fp = stdin;
    int mode;
    int argn;
    unsigned char bf[3];
    pixel *pixrow;

    ppm_init(&argc, argv);
    argn = 1;
    if (argn != argc)
	fp = pm_openr(argv[argn++]);
    else
	fp = stdin;

    if (argn != argc)
	pm_usage(usage);

    while ((c = fgetc(fp)) != -1) {
	if (c != '\033')
	    continue;
	switch (c = egetc(fp)) {
	case 'E':	/* reset */
	    break;
	case '*':
	    cmd = egetc(fp);
	    for (i = 0; i < BUFSIZ; i++) {
		if (!isdigit(c = egetc(fp)) && c != '+' && c != '-')
		    break;
		buffer[i] = c;
	    }
	    if (i != 0) {
		buffer[i] = '\0';
		if (sscanf(buffer, "%d", &val) != 1) 
		    pm_error("bad value `%s' at <ESC>*%c%c", buffer, cmd, c);
	    }
	    else
		val = -1;
	    switch (cmd) {
	    case 't':
		switch (c) {
		case 'J':	/* render */
		    break;
		case 'K':	/* back scale */
		    break;
		case 'I':	/* gamma */
		    break;
		case 'R':
		    break;	/* set resolution */
		default:
		    pm_message("uninmplemented <ESC>*%c%d%c", cmd, val, c);
		    break;
		}
		break;
	    case 'r':
		switch (c) {
		case 'S':	/* width */
		    cols = val;
		    break;
		case 'T':	/* height */
		    rows = val;
		    break;
		case 'U':	/* planes */
		    planes = val;
		    if (planes != 3) 
			pm_error("can handle only 3 plane files");
		    break;
		case 'A':	/* begin raster */
		    break;
		case 'B':
		case 'C':	/* end raster */
		    break;
		case 'V':
		    break;	/* set deci height */
		case 'H':
		    break;	/* set deci width */
		default:
		    pm_message("uninmplemented <ESC>*%c%d%c", cmd, val, c);
		    break;
		}
		break;
	    case 'b':
		switch (c) {
		case 'M':	/* transmission mode */
		    if (val != 0 && val != 1)
			pm_error("unimplemented trasmission mode %d", val);
		    mode = val;
		    break;
		case 'V':	/* send plane */
		case 'W':	/* send last plane */
		    if (rows == -1 || r >= rows || image == NULL) {
			if (rows == -1 || r >= rows)
			    rows += 100;
			if (image == NULL) {
			    image = (unsigned char **) 
				malloc(rows * planes * sizeof(unsigned char *));
			    imlen = (int *) malloc(rows * planes * sizeof(int));
			}
			else {
			    image = (unsigned char **) 
				realloc(image, rows * planes * 
					sizeof(unsigned char *));
			    imlen = (int *) realloc(imlen, rows * planes * 
						    sizeof(int));
			}
		    }
		    if (image == NULL || imlen == NULL)
			pm_error("out of memory");
		    if (p == planes) 
			pm_error("too many planes");
		    cols = cols > val ? cols : val;
		    imlen[r * planes + p] = val;
		    image[r * planes + p] = (unsigned char *) 
			malloc(val * sizeof(unsigned char));
		    if (image[r * planes + p] == NULL) 
			pm_error("out of memory");
		    if (fread(image[r * planes + p], 1, val, fp) != val) 
			pm_error("short data");
		    if (c == 'V')
			p++;
		    else {
			p = 0;
			r++;
		    }
		    break;
		default:
		    pm_message("uninmplemented <ESC>*%c%d%c", cmd, val, c);
		    break;
		}
		break;
	    case 'p': /* Position */
		if (p != 0) 
		    pm_error("changed position in the middle of transferring planes");
		switch (c) {
		case 'X':
		    pm_message("can only position in y");
		    break;
		case 'Y':
		    if (buffer[0] == '+')
			val = r + val;
		    if (buffer[0] == '-')
			val = r - val;
		    for (; val > r; r++) 
			for (p = 0; p < 3; p++) {
			    imlen[r * planes + p] = 0;
			    image[r * planes + p] = NULL;
			}
		    r = val;
		    break;
		default:
		    pm_message("uninmplemented <ESC>*%c%d%c", cmd, val, c);
		    break;
		}
	    default:
		pm_message("uninmplemented <ESC>*%c%d%c", cmd, val, c);
		break;
	    }
	}
    }
    pm_close(fp);
    rows = r;
    if (mode == 1) {
	unsigned char *buf;
	int newcols = 0;
	newcols = 10240; /* It could not be larger that that! */
	cols = 0;
	for (r = 0; r < rows; r++) {
	    if (image[r * planes] == NULL)
		continue;
	    for (p = 0; p < planes; p++) {
		buf = (unsigned char *) malloc(newcols * 
					       sizeof(unsigned char *));
		if (buf == NULL) 
		    pm_error("out of memory");
		for (i = 0, c = 0; c < imlen[p + r * planes]; c += 2)
		    for (cmd = image[p + r * planes][c],
			 val = image[p + r * planes][c+1]; 
			 cmd >= 0 && i < newcols; cmd--, i++) 
			 buf[i] = val;
		cols = cols > i ? cols : i;
		free(image[p + r * planes]);
		/* 
		 * This is less than what we have so it realloc should 
		 * not return null. Even if it does, tough! We will
		 * lose a line, and probably die on the next line anyway
		 */
		image[p + r * planes] = (unsigned char *) realloc(buf, i);
	    }
	}
	cols *= 8;
    }
			
	   
    ppm_writeppminit(stdout, cols, rows, (pixval) 255, 0);
    pixrow = ppm_allocrow(cols);
    for (r = 0; r < rows; r++) {
	if (image[r * planes] == NULL) {
	    for (c = 0; c < cols; c++)
		PPM_ASSIGN(pixrow[c], 0, 0, 0);
	    continue;
	}
	for (cmd = 0, c = 0; c < cols; c += 8, cmd++) 
	    for (i = 0; i < 8 & c + i < cols; i++) {
		for (p = 0; p < planes; p++) 
		    if (mode == 0 && cmd >= imlen[r * planes + p])
			bf[p] = 0;
		    else
			bf[p] = (image[r * planes + p][cmd] & 
				     (1 << (7 - i))) ? 255 : 0;
		PPM_ASSIGN(pixrow[c + i], bf[0], bf[1], bf[2]);
	    }
	    ppm_writeppmrow(stdout, pixrow, cols, (pixval) 255, 0);
    }
    pm_close(stdout);
    exit(0);
}
