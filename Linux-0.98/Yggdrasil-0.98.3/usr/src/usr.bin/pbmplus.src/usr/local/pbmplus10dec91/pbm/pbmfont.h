/* pbmfont.h - header file for font routines in libpbm
*/

bit** pbm_defaultfont ARGS(( int* fcolsP, int* frowsP ));

void pbm_dissectfont ARGS(( bit** font, int frows, int fcols, int* char_heightP, int* char_widthP, int* char_aheightP, int* char_awidthP, int char_row0[], int char_col0[] ));

void pbm_dumpfont ARGS(( bit** font, int fcols, int frows ));
