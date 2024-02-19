#ifndef _EXT_FS_SB
#define _EXT_FS_SB

/*
 * extended-fs super-block data in memory
 */
struct ext_sb_info {
			unsigned long s_ninodes;
			unsigned long s_nzones;
			unsigned long s_firstdatazone;
			unsigned long s_log_zone_size;
			unsigned long s_max_size;
			unsigned long s_firstfreeblocknumber;
			unsigned long s_freeblockscount;
			struct buffer_head * s_firstfreeblock;
			unsigned long s_firstfreeinodenumber;
			unsigned long s_freeinodescount;
			struct buffer_head * s_firstfreeinodeblock;
};

#endif
