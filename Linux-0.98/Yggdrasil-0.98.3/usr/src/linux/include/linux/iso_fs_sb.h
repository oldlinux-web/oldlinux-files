#ifndef _ISOFS_FS_SB
#define _ISOFS_FS_SB

/*
 * minix super-block data in memory
 */
struct isofs_sb_info {
			unsigned long s_ninodes;
			unsigned long s_nzones;
			unsigned long s_firstdatazone;
			unsigned long s_log_zone_size;
			unsigned long s_max_size;

			unsigned char s_high_sierra; /* A simple flag */
			unsigned char s_mapping;
			unsigned char s_conversion;
			unsigned char s_rock;
};

#endif







