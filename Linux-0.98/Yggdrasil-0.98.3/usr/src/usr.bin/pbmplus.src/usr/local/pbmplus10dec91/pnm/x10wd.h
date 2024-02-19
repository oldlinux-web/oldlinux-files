/* x10wd.h - the following defs are taken from various X10 header files
*/

#ifndef _X10WD_H_
#define _X10WD_H_

#define XYFormat 0
#define ZFormat 1

#define X10WD_FILE_VERSION 6
typedef struct {
    int header_size;		/* Size of the entire file header (bytes). */
    int file_version;		/* X10WD_FILE_VERSION */
    int display_type;		/* Display type. */
    int display_planes;		/* Number of display planes. */
    int pixmap_format;		/* Pixmap format. */
    int pixmap_width;		/* Pixmap width. */
    int pixmap_height;		/* Pixmap height. */
    short window_width;		/* Window width. */
    short window_height;	/* Window height. */
    short window_x;		/* Window upper left X coordinate. */
    short window_y;		/* Window upper left Y coordinate. */
    short window_bdrwidth;	/* Window border width. */
    short window_ncolors;	/* number of Color entries in this window */
    } X10WDFileHeader;

typedef struct {
    int pixel;
    unsigned short red, green, blue;
    } X10Color;

#endif /*_X10WD_H_*/
