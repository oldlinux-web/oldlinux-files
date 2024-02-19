/* VGAlib version 1.0 - (c) 1992 Tommy Frandsen 		   */
/*								   */
/* This library is free software; you can redistribute it and/or   */
/* modify it without any restrictions. This library is distributed */
/* in the hope that it will be useful, but without any warranty;   */
/* without even the implied warranty of merchantability or fitness */
/* for a particular purpose.					   */

#ifndef VGA_H
#define VGA_H

#define TEXT 	     0
#define G320x200x16  1
#define G640x200x16  2
#define G640x350x16  3
#define G640x480x16  4
#define G320x200x256 5
#define G320x240x256 6
#define G320x400x256 7
#define G360x480x256 8

extern int vga_setmode(int mode);
extern int vga_setflipchar(int c);

extern int vga_clear();
extern int vga_flip();

extern int vga_getxdim();
extern int vga_getydim();
extern int vga_getcolors();

extern int vga_setpalette(int index, int red, int green, int blue);
extern int vga_getpalette(int index, int *red, int *green, int *blue);
extern int vga_setpalvec(int start, int num, int *pal);
extern int vga_getpalvec(int start, int num, int *pal);

extern int vga_screenoff();
extern int vga_screenon();

extern int inline vga_setcolor(int color);
extern int inline vga_drawpixel(int x, int y);
extern int vga_drawline(int x1, int y1, int x2, int y2);
extern int vga_drawscanline(int line, char* colors);

extern int vga_getch();

#endif /* VGA_H */

