/*
 *  linux/fs/ext/freelists.c
 *
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *
 */

/* freelists.c contains the code that handles the inode and block free lists */


/*

   The free blocks are managed by a linked list. The super block contains the
   number of the first free block. This block contains 254 numbers of other
   free blocks and the number of the next block in the list.

   When an ext fs is mounted, the number of the first free block is stored
   in s->u.ext_sb.s_firstfreeblocknumber and the block header is stored in
   s->u.ext_sb.s_firstfreeblock. u.ext_sb.s_freeblockscount contains the count
   of free blocks.

   The free inodes are also managed by a linked list in a similar way. The
   super block contains the number of the first free inode. This inode contains
   14 numbers of other free inodes and the number of the next inode in the list.
   
   The number of the first free inode is stored in
   s->u.ext_sb.s_firstfreeinodenumber and the header of the block containing
   the inode is stored in s->u.ext_sb.s_firstfreeinodeblock.
   u.ext_sb.s_freeinodescount contains the count of free inodes.

*/

#include <linux/sched.h>
#include <linux/ext_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/locks.h>

#define clear_block(addr) \
__asm__("cld\n\t" \
        "rep\n\t" \
        "stosl" \
        ::"a" (0),"c" (BLOCK_SIZE/4),"D" ((long) (addr)):"cx","di")

void ext_free_block(struct super_block * sb, int block)
{
	struct buffer_head * bh;
	struct ext_free_block * efb;

	if (!sb)
		panic("trying to free block on nonexistent device");
	lock_super (sb);
	if (block < sb->u.ext_sb.s_firstdatazone ||
	    block >= sb->u.ext_sb.s_nzones) {
		printk("trying to free block not in datazone\n");
		return;
	}
	bh = get_hash_table(sb->s_dev, block, sb->s_blocksize);
	if (bh)
		bh->b_dirt=0;
	brelse(bh);
	if (sb->u.ext_sb.s_firstfreeblock)
		efb = (struct ext_free_block *) sb->u.ext_sb.s_firstfreeblock->b_data;
	if (!sb->u.ext_sb.s_firstfreeblock || efb->count == 254) {
#ifdef EXTFS_DEBUG
printk("ext_free_block: block full, skipping to %d\n", block);
#endif
		if (sb->u.ext_sb.s_firstfreeblock)
			brelse (sb->u.ext_sb.s_firstfreeblock);
		if (!(sb->u.ext_sb.s_firstfreeblock = bread (sb->s_dev,
			block, sb->s_blocksize)))
			panic ("ext_free_block: unable to read block to free\n");
		efb = (struct ext_free_block *) sb->u.ext_sb.s_firstfreeblock->b_data;
		efb->next = sb->u.ext_sb.s_firstfreeblocknumber;
		efb->count = 0;
		sb->u.ext_sb.s_firstfreeblocknumber = block;
	} else {
		efb->free[efb->count++] = block;
	}
	sb->u.ext_sb.s_freeblockscount ++;
	sb->s_dirt = 1;
	sb->u.ext_sb.s_firstfreeblock->b_dirt = 1;
	unlock_super (sb);
	return;
}

int ext_new_block(struct super_block * sb)
{
	struct buffer_head * bh;
	struct ext_free_block * efb;
	int j;

	if (!sb)
		panic("trying to get new block from nonexistant device");
	if (!sb->u.ext_sb.s_firstfreeblock)
		return 0;
	lock_super (sb);
	efb = (struct ext_free_block *) sb->u.ext_sb.s_firstfreeblock->b_data;
	if (efb->count) {
		j = efb->free[--efb->count];
		sb->u.ext_sb.s_firstfreeblock->b_dirt = 1;
	} else {
#ifdef EXTFS_DEBUG
printk("ext_new_block: block empty, skipping to %d\n", efb->next);
#endif
		j = sb->u.ext_sb.s_firstfreeblocknumber;
		sb->u.ext_sb.s_firstfreeblocknumber = efb->next;
		brelse (sb->u.ext_sb.s_firstfreeblock);
		if (!sb->u.ext_sb.s_firstfreeblocknumber) {
			sb->u.ext_sb.s_firstfreeblock = NULL;
		} else {
			if (!(sb->u.ext_sb.s_firstfreeblock = bread (sb->s_dev,
				sb->u.ext_sb.s_firstfreeblocknumber,
				sb->s_blocksize)))
				panic ("ext_new_block: unable to read next free block\n");
		}
	}
	if (j < sb->u.ext_sb.s_firstdatazone || j > sb->u.ext_sb.s_nzones) {
		printk ("ext_new_block: blk = %d\n", j);
		panic ("allocating block not in data zone\n");
	}
	sb->u.ext_sb.s_freeblockscount --;
	sb->s_dirt = 1;

	if (!(bh=getblk(sb->s_dev, j, sb->s_blocksize)))
		panic("new_block: cannot get block");
	if (bh->b_count != 1)
		panic("new block: count is != 1");
	clear_block(bh->b_data);
	bh->b_uptodate = 1;
	bh->b_dirt = 1;
	brelse(bh);
#ifdef EXTFS_DEBUG
printk("ext_new_block: allocating block %d\n", j);
#endif
	unlock_super (sb);
	return j;
}

unsigned long ext_count_free_blocks(struct super_block *sb)
{
#ifdef EXTFS_DEBUG
	struct buffer_head * bh;
	struct ext_free_block * efb;
	unsigned long count, block;

	lock_super (sb);
	if (!sb->u.ext_sb.s_firstfreeblock)
		count = 0;
	else {
		efb = (struct ext_free_block *) sb->u.ext_sb.s_firstfreeblock->b_data;
		count = efb->count + 1;
		block = efb->next;
		while (block) {
			if (!(bh = bread (sb->s_dev, block, sb->s_blocksize))) {
				printk ("ext_count_free: error while reading free blocks list\n");
				block = 0;
			} else {
				efb = (struct ext_free_block *) bh->b_data;
				count += efb->count + 1;
				block = efb->next;
				brelse (bh);
			}
		}
	}
printk("ext_count_free_blocks: stored = %d, computed = %d\n",
	sb->u.ext_sb.s_freeblockscount, count);
	unlock_super (sb);
	return count;
#else
	return sb->u.ext_sb.s_freeblockscount;
#endif
}

void ext_free_inode(struct inode * inode)
{
	struct buffer_head * bh;
	struct ext_free_inode * efi;
	unsigned long block;

	if (!inode)
		return;
	if (!inode->i_dev) {
		memset(inode,0,sizeof(*inode));
		return;
	}
	if (inode->i_count>1) {
		printk("free_inode: inode has count=%d\n",inode->i_count);
		return;
	}
	if (inode->i_nlink) {
		printk("free_inode: inode has nlink=%d\n",inode->i_nlink);
		return;
	}
	if (!inode->i_sb) {
		printk("free_inode: inode on nonexistent device\n");
		return;
	}
	lock_super (inode->i_sb);
	if (inode->i_ino < 1 || inode->i_ino > inode->i_sb->u.ext_sb.s_ninodes) {
		printk("free_inode: inode 0 or nonexistent inode\n");
		unlock_super (inode->i_sb);
		return;
	}
	if (inode->i_sb->u.ext_sb.s_firstfreeinodeblock)
		efi = ((struct ext_free_inode *) inode->i_sb->u.ext_sb.s_firstfreeinodeblock->b_data) +
			(inode->i_sb->u.ext_sb.s_firstfreeinodenumber-1)%EXT_INODES_PER_BLOCK;
	if (!inode->i_sb->u.ext_sb.s_firstfreeinodeblock || efi->count == 14) {
#ifdef EXTFS_DEBUG
printk("ext_free_inode: inode full, skipping to %d\n", inode->i_ino);
#endif
		if (inode->i_sb->u.ext_sb.s_firstfreeinodeblock)
			brelse (inode->i_sb->u.ext_sb.s_firstfreeinodeblock);
		block = 2 + (inode->i_ino - 1) / EXT_INODES_PER_BLOCK;
		if (!(bh = bread(inode->i_dev, block, inode->i_sb->s_blocksize)))
			panic("ext_free_inode: unable to read inode block\n");
		efi = ((struct ext_free_inode *) bh->b_data) +
			(inode->i_ino - 1) % EXT_INODES_PER_BLOCK;
		efi->next = inode->i_sb->u.ext_sb.s_firstfreeinodenumber;
		efi->count = 0;
		inode->i_sb->u.ext_sb.s_firstfreeinodenumber = inode->i_ino;
		inode->i_sb->u.ext_sb.s_firstfreeinodeblock = bh;
	} else {
		efi->free[efi->count++] = inode->i_ino;
	}
	inode->i_sb->u.ext_sb.s_freeinodescount ++;
	inode->i_sb->s_dirt = 1;
	inode->i_sb->u.ext_sb.s_firstfreeinodeblock->b_dirt = 1;
	unlock_super (inode->i_sb);
	memset(inode,0,sizeof(*inode));
}

struct inode * ext_new_inode(struct super_block * sb)
{
	struct inode * inode;
	struct ext_free_inode * efi;
	unsigned long block;
	int j;

	if (!sb || !(inode=get_empty_inode()))
		return NULL;
	inode->i_sb = sb;
	inode->i_flags = sb->s_flags;
	if (!sb->u.ext_sb.s_firstfreeinodeblock)
		return 0;
	lock_super (sb);
	efi = ((struct ext_free_inode *) sb->u.ext_sb.s_firstfreeinodeblock->b_data) +
		(sb->u.ext_sb.s_firstfreeinodenumber-1)%EXT_INODES_PER_BLOCK;
	if (efi->count) {
		j = efi->free[--efi->count];
		sb->u.ext_sb.s_firstfreeinodeblock->b_dirt = 1;
	} else {
#ifdef EXTFS_DEBUG
printk("ext_free_inode: inode empty, skipping to %d\n", efi->next);
#endif
		j = sb->u.ext_sb.s_firstfreeinodenumber;
		if (efi->next > sb->u.ext_sb.s_ninodes) {
			printk ("efi->next = %d\n", efi->next);
			panic ("ext_new_inode: bad inode number in free list\n");
		}
		sb->u.ext_sb.s_firstfreeinodenumber = efi->next;
		block = 2 + (((unsigned long) efi->next) - 1) / EXT_INODES_PER_BLOCK;
		brelse (sb->u.ext_sb.s_firstfreeinodeblock);
		if (!sb->u.ext_sb.s_firstfreeinodenumber) {
			sb->u.ext_sb.s_firstfreeinodeblock = NULL;
		} else {
			if (!(sb->u.ext_sb.s_firstfreeinodeblock =
			    bread(sb->s_dev, block, sb->s_blocksize)))
				panic ("ext_new_inode: unable to read next free inode block\n");
		}
	}
	sb->u.ext_sb.s_freeinodescount --;
	sb->s_dirt = 1;
	inode->i_count = 1;
	inode->i_nlink = 1;
	inode->i_dev = sb->s_dev;
	inode->i_uid = current->euid;
	inode->i_gid = current->egid;
	inode->i_dirt = 1;
	inode->i_ino = j;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = NULL;
	inode->i_blocks = inode->i_blksize = 0;
#ifdef EXTFS_DEBUG
printk("ext_new_inode : allocating inode %d\n", inode->i_ino);
#endif
	unlock_super (sb);
	return inode;
}

unsigned long ext_count_free_inodes(struct super_block *sb)
{
#ifdef EXTFS_DEBUG
	struct buffer_head * bh;
	struct ext_free_inode * efi;
	unsigned long count, block, ino;

	lock_super (sb);
	if (!sb->u.ext_sb.s_firstfreeinodeblock)
		count = 0;
	else {
		efi = ((struct ext_free_inode *) sb->u.ext_sb.s_firstfreeinodeblock->b_data) +
			((sb->u.ext_sb.s_firstfreeinodenumber-1)%EXT_INODES_PER_BLOCK);
		count = efi->count + 1;
		ino = efi->next;
		while (ino) {
			if (ino < 1 || ino > sb->u.ext_sb.s_ninodes) {
				printk ("u.ext_sb.s_firstfreeinodenumber = %d, ino = %d\n", 
					(int) sb->u.ext_sb.s_firstfreeinodenumber,ino);
				panic ("ext_count_fre_inodes: bad inode number in free list\n");
			}
			block = 2 + ((ino - 1) / EXT_INODES_PER_BLOCK);
			if (!(bh = bread (sb->s_dev, block, sb->s_blocksize))) {
				printk ("ext_count_free_inodes: error while reading free inodes list\n");
				block = 0;
			} else {
				efi = ((struct ext_free_inode *) bh->b_data) +
					((ino - 1) % EXT_INODES_PER_BLOCK);
				count += efi->count + 1;
				ino = efi->next;
				brelse (bh);
			}
		}
	}
printk("ext_count_free_inodes: stored = %d, computed = %d\n",
	sb->u.ext_sb.s_freeinodescount, count);
	unlock_super (sb);
	return count;
#else
	return sb->u.ext_sb.s_freeinodescount;
#endif
}
