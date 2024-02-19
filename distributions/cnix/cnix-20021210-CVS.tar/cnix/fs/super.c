#include <cnix/ext2_fs.h>
#include <cnix/partition.h>
#include <string.h>

extern void lba_read_sector(long sector,unsigned char *buf);
extern void get_partition(void);

extern struct disk_partition part_p[8]; 
struct partition *ext2_part;

char file_buf[100*512]; /*no malloc function to test my define 50k memory*/

struct cnix_ext2 cn_e2;

read_desc(int count,char *buf,struct ext2_group_desc *e2gd)
{
        int offset_sector;
	int offset_count;
	
	/*find the group block at group describe position
	 * get the group bolck start inode table 
	 */

	offset_sector = count / (512/32);
	offset_count = count %(512/32);
	  
	lba_read_sector(cn_e2.start_sect +cn_e2.block_size+offset_sector,buf);
	e2gd =&buf[ (offset_count-1) *32 ];
	

}
int 
read_super(char *s)
{
        unsigned char buf[512];
        struct ext2_super_block *e2sb;
	struct ext2_group_desc  *e2gd;
	struct ext2_inode e2ino;
	struct ext2_inode e2ino_root;
	struct ext2_dir_entry_2 *e2dir;
	

	char *filename;
	int block_size;
	static int address= 0;
	int i;

	for(i=0;i<8;i++){
	     if(strcmp(s,&part_p[i].name)==0)
	           ext2_part =&part_p[i].part;
	}

	if(ext2_part->sys_ind!=0x83) return 0;
	
       	lba_read_sector(ext2_part->start_sect +2,buf); /*first sector is boot block ,so 2nd sector is super!*/
	e2sb = &buf[0];

	if(e2sb->s_magic == EXT2_SUPER_MAGIC )
	        printk("Find ext2 super block ok!\n");
	else { printk("Can't find ext2 super block!\n"); return 0;}

	block_size = 1<<e2sb->s_log_block_size ;/*fix by asmcos,thank xiexiecn*/
	/*save the super block data to cnix ext2 for call*/
	cn_e2.block_size = block_size *2;
	cn_e2.s_blocks_per_group = e2sb->s_blocks_per_group;
	cn_e2.s_inodes_per_group = e2sb->s_inodes_per_group;
	cn_e2.s_inode_size = e2sb->s_inode_size;
	cn_e2.start_sect = ext2_part->start_sect;
	/*group describe table
	* the boot and super block in 0 block
	* the group describ table in 1 block
	* the block size = n k(n = 1,2,4), = 2n sectors.
	*/


	read_inode(2,buf,&e2ino);

	lba_read_sector(cn_e2.start_sect +e2ino.i_block[0] * cn_e2.block_size ,buf);

	get_root_inode(&buf);

	printk("Your root dir:\n");
	{int i =0;
	while(cn_e2.root_dir[i].inode)
	  {
	    print_fn(&cn_e2.root_dir[i].name);
	    i++;
	  }	
	printk("\n");
	}
}
print_fn(char *filename)
{
  	printk("/");
        printk("%s",filename);
	printk("     ");

}


fs_init()
{
         get_partition();
	 read_super("hda2");
}
