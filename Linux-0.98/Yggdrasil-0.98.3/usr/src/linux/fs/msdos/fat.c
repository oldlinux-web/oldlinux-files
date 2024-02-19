/*
 *  linux/fs/msdos/fat.c
 *
 *  Written 1992 by Werner Almesberger
 */

#include <linux/msdos_fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stat.h>


static struct fat_cache *fat_cache,cache[FAT_CACHE];

/* Returns the this'th FAT entry, -1 if it is an end-of-file entry. If
   new_value is != -1, that FAT entry is replaced by it. */

int fat_access(struct super_block *sb,int this,int new_value)
{
	struct buffer_head *bh,*bh2,*c_bh,*c_bh2;
	unsigned char *p_first,*p_last;
	void *data,*data2,*c_data,*c_data2;
	int first,last,next,copy;

	if (MSDOS_SB(sb)->fat_bits == 16) first = last = this*2;
	else {
		first = this*3/2;
		last = first+1;
	}
	if (!(bh = msdos_sread(sb->s_dev,MSDOS_SB(sb)->fat_start+(first >>
	    SECTOR_BITS),&data))) {
		printk("bread in fat_access failed\n");
		return 0;
	}
	if ((first >> SECTOR_BITS) == (last >> SECTOR_BITS)) {
		bh2 = bh;
		data2 = data;
	}
	else {
		if (!(bh2 = msdos_sread(sb->s_dev,MSDOS_SB(sb)->fat_start+(last
		    >> SECTOR_BITS),&data2))) {
			brelse(bh);
			printk("bread in fat_access failed\n");
			return 0;
		}
	}
	if (MSDOS_SB(sb)->fat_bits == 16) {
		p_first = p_last = NULL; /* GCC needs that stuff */
		next = ((unsigned short *) data)[(first & (SECTOR_SIZE-1))
		    >> 1];
		if (next >= 0xfff8) next = -1;
	}
	else {
		p_first = &((unsigned char *) data)[first & (SECTOR_SIZE-1)];
		p_last = &((unsigned char *) data2)[(first+1) &
		    (SECTOR_SIZE-1)];
		if (this & 1) next = ((*p_first >> 4) | (*p_last << 4)) & 0xfff;
		else next = (*p_first+(*p_last << 8)) & 0xfff;
		if (next >= 0xff8) next = -1;
	}
	if (new_value != -1) {
		if (MSDOS_SB(sb)->fat_bits == 16)
			((unsigned short *) data)[(first & (SECTOR_SIZE-1)) >>
			    1] = new_value;
		else {
			if (this & 1) {
				*p_first = (*p_first & 0xf) | (new_value << 4);
				*p_last = new_value >> 4;
			}
			else {
				*p_first = new_value & 0xff;
				*p_last = (*p_last & 0xf0) | (new_value >> 8);
			}
			bh2->b_dirt = 1;
		}
		bh->b_dirt = 1;
		for (copy = 1; copy < MSDOS_SB(sb)->fats; copy++) {
			if (!(c_bh = msdos_sread(sb->s_dev,MSDOS_SB(sb)->
			    fat_start+(first >> SECTOR_BITS)+MSDOS_SB(sb)->
			    fat_length*copy,&c_data))) break;
			memcpy(c_data,data,SECTOR_SIZE);
			c_bh->b_dirt = 1;
			if (data != data2 || bh != bh2) {
				if (!(c_bh2 = msdos_sread(sb->s_dev,
				    MSDOS_SB(sb)->fat_start+(first >>
				    SECTOR_BITS)+MSDOS_SB(sb)->fat_length*copy
				    +1,&c_data2))) {
					brelse(c_bh);
					break;
				}
				memcpy(c_data2,data2,SECTOR_SIZE);
				brelse(c_bh2);
			}
			brelse(c_bh);
		}
	}
	brelse(bh);
	if (data != data2) brelse(bh2);
	return next;
}


void cache_init(void)
{
	static int initialized = 0;
	int count;

	if (initialized) return;
	fat_cache = &cache[0];
	for (count = 0; count < FAT_CACHE; count++) {
		cache[count].device = 0;
		cache[count].next = count == FAT_CACHE-1 ? NULL :
		    &cache[count+1];
	}
	initialized = 1;
}


void cache_lookup(struct inode *inode,int cluster,int *f_clu,int *d_clu)
{
	struct fat_cache *walk;

#ifdef DEBUG
printk("cache lookup: <%d,%d> %d (%d,%d) -> ",inode->i_dev,inode->i_ino,cluster,
    *f_clu,*d_clu);
#endif
	for (walk = fat_cache; walk; walk = walk->next)
		if (inode->i_dev == walk->device && walk->ino == inode->i_ino &&
		    walk->file_cluster <= cluster && walk->file_cluster >
		    *f_clu) {
			*d_clu = walk->disk_cluster;
#ifdef DEBUG
printk("cache hit: %d (%d)\n",walk->file_cluster,*d_clu);
#endif
			if ((*f_clu = walk->file_cluster) == cluster) return;
		}
#ifdef DEBUG
printk("cache miss\n");
#endif
}


#ifdef DEBUG
static void list_cache(void)
{
	struct fat_cache *walk;

	for (walk = fat_cache; walk; walk = walk->next) {
		if (walk->device)
			printk("<%d,%d>(%d,%d) ",walk->device,walk->ino,
			    walk->file_cluster,walk->disk_cluster);
		else printk("-- ");
	}
	printk("\n");
}
#endif


void cache_add(struct inode *inode,int f_clu,int d_clu)
{
	struct fat_cache *walk,*last;

#ifdef DEBUG
printk("cache add: <%d,%d> %d (%d)\n",inode->i_dev,inode->i_ino,f_clu,d_clu);
#endif
	last = NULL;
	for (walk = fat_cache; walk->next; walk = (last = walk)->next)
		if (inode->i_dev == walk->device && walk->ino == inode->i_ino &&
		    walk->file_cluster == f_clu) {
			if (walk->disk_cluster != d_clu)
				panic("FAT cache corruption");
			/* update LRU */
			if (last == NULL) return;
			last->next = walk->next;
			walk->next = fat_cache;
			fat_cache = walk;
#ifdef DEBUG
list_cache();
#endif
			return;
		}
	walk->device = inode->i_dev;
	walk->ino = inode->i_ino;
	walk->file_cluster = f_clu;
	walk->disk_cluster = d_clu;
	last->next = NULL;
	walk->next = fat_cache;
	fat_cache = walk;
#ifdef DEBUG
list_cache();
#endif
}


/* Cache invalidation occurs rarely, thus the LRU chain is not updated. It
   fixes itself after a while. */

void cache_inval_inode(struct inode *inode)
{
	struct fat_cache *walk;

	for (walk = fat_cache; walk; walk = walk->next)
		if (walk->device == inode->i_dev && walk->ino == inode->i_ino)
			walk->device = 0;
}


void cache_inval_dev(int device)
{
	struct fat_cache *walk;

	for (walk = fat_cache; walk; walk = walk->next)
		if (walk->device == device) walk->device = 0;
}


int get_cluster(struct inode *inode,int cluster)
{
	int this,count;

	if (!(this = MSDOS_I(inode)->i_start)) return 0;
	if (!cluster) return this;
	count = 0;
	for (cache_lookup(inode,cluster,&count,&this); count < cluster;
	    count++) {
		if ((this = fat_access(inode->i_sb,this,-1)) == -1) return 0;
		if (!this) return 0;
	}
	if (!(MSDOS_I(inode)->i_busy && inode->i_nlink))
		cache_add(inode,cluster,this);
	/* don't add clusters of moved files, because we can't invalidate them
	   when this inode is returned. */
	return this;
}


int msdos_smap(struct inode *inode,int sector)
{
	struct msdos_sb_info *sb;
	int cluster,offset;

	sb = MSDOS_SB(inode->i_sb);
	if (inode->i_ino == MSDOS_ROOT_INO || (S_ISDIR(inode->i_mode) &&
	    !MSDOS_I(inode)->i_start)) {
		if (sector >= sb->dir_entries >> MSDOS_DPS_BITS) return 0;
		return sector+sb->dir_start;
	}
	cluster = sector/sb->cluster_size;
	offset = sector % sb->cluster_size;
	if (!(cluster = get_cluster(inode,cluster))) return 0;
	return (cluster-2)*sb->cluster_size+sb->data_start+offset;
}


/* Free all clusters after the skip'th cluster. Doesn't use the cache,
   because this way we get an additional sanity check. */

int fat_free(struct inode *inode,int skip)
{
	int this,last;

	if (!(this = MSDOS_I(inode)->i_start)) return 0;
	last = 0;
	while (skip--) {
		last = this;
		if ((this = fat_access(inode->i_sb,this,-1)) == -1) return 0;
		if (!this) {
			printk("fat_free: skipped EOF\n");
			return -EIO;
		}
	}
	if (last)
		fat_access(inode->i_sb,last,MSDOS_SB(inode->i_sb)->fat_bits ==
		    12 ? 0xff8 : 0xfff8);
	else {
		MSDOS_I(inode)->i_start = 0;
		inode->i_dirt = 1;
	}
	lock_fat(inode->i_sb);
	while (this != -1) {
		if (!(this = fat_access(inode->i_sb,this,0)))
			panic("fat_free: deleting beyond EOF");
		if (MSDOS_SB(inode->i_sb)->free_clusters != -1)
			MSDOS_SB(inode->i_sb)->free_clusters++;
		inode->i_blocks--;
	}
	unlock_fat(inode->i_sb);
	cache_inval_inode(inode);
	return 0;
}
