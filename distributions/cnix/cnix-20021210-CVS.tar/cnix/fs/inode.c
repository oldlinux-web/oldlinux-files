#include <cnix/fs.h>
#include <cnix/ext2_fs.h>

extern struct cnix_ext2 cn_e2;
struct ext2_dir_entry_2 current_dir[20];



get_root_inode(char *buf)
{
        int address;
	char *filename;
	int i=0,j;
        struct ext2_dir_entry_2 *e2dir;


	address = &buf[0];
	e2dir = &buf[0];
	while(e2dir->inode){
	        
	       
		filename = &e2dir->name[0];
		handler_dir(filename,e2dir->name_len,e2dir);
		
		if( (e2dir->name_len%4) ==0)
		  address = address +8 + e2dir->name_len;
	        else address = address +8 + (e2dir->name_len /4 +1 )*4;
		
		e2dir = address;
         }

}
static int root_count=0;

handler_dir(char *filename,int name_len,struct ext2_dir_entry_2 *e2dir)
{
        char tmp;
	int i;
        tmp = filename[name_len];
	filename[name_len]='\0';
	cn_e2.root_dir[root_count] = *e2dir;

	for(i = 0;i<name_len+1;i++)
	       cn_e2.root_dir[root_count].name[i] = filename[i];

	root_count ++;
       
       	filename[name_len] = tmp;

}


read_inode(int count,char *buf_1,struct ext2_inode *e2ino_1)
{       
  	struct ext2_group_desc  *e2gd;
	struct ext2_inode *e2ino;
	int group_block;
	int offset_group_block;
	unsigned char buf_gd[512];
	unsigned char buf[512];

	group_block = count /cn_e2.s_inodes_per_group;
	offset_group_block = count % cn_e2.s_inodes_per_group;
	group_block ++;
	


{
        int offset_sector;
	int offset_count;
	int gd_offset;
	
	/*find the group block at group describe position
	 * get the group bolck start inode table 
	 */

	offset_sector = group_block / (512/32);
	offset_count = group_block %(512/32);
	
	if(cn_e2.block_size==2) gd_offset = 1;  
	else gd_offset = 0;

	lba_read_sector(cn_e2.start_sect +(gd_offset+1)*cn_e2.block_size+offset_sector,buf_gd);
	e2gd =&buf_gd[ (offset_count-1) *32 ];

}


#if 0
	read_desc(group_block,buf,e2gd);
	
	read_inode_offset(offset_group_block,e2gd->bg_inode_table,buf,e2ino);
		
#endif
{
        int inode_block;
	int offset;
	int sector,offset_sector;
	
	/*512/cn_e2.s_inode_size = count inode at per sector */
        inode_block = offset_group_block / (cn_e2.block_size *(512/cn_e2.s_inode_size));
	offset = offset_group_block % (cn_e2.block_size *(512/cn_e2.s_inode_size));
	sector = offset /(512/cn_e2.s_inode_size);
	offset_sector = (offset-1) % (512/cn_e2.s_inode_size);
   
        lba_read_sector(cn_e2.start_sect +e2gd->bg_inode_table * cn_e2.block_size +inode_block*cn_e2.block_size+sector,buf);
  
	e2ino = & buf[offset_sector * cn_e2.s_inode_size];

}
        {    int i ;
	     for (i=0;i<15;i++)
	          e2ino_1->i_block[i]=e2ino->i_block[i];
        }
        e2ino_1->i_size = e2ino->i_size;
	
}

read_inode_offset(int count,unsigned long inode_table,char *buf,struct ext2_inode *e2ino)
{
        int inode_block;
	int offset;
	int sector,offset_sector;
	
	/*512/cn_e2.s_inode_size = count inode at per sector */
        inode_block = count / (cn_e2.block_size*(512/cn_e2.s_inode_size));
	offset = count % (cn_e2.block_size*(512/cn_e2.s_inode_size));
	sector = offset /(512/cn_e2.s_inode_size);
	offset_sector = (offset-1) % (512/cn_e2.s_inode_size);
   
        lba_read_sector(cn_e2.start_sect +inode_table * cn_e2.block_size +inode_block*cn_e2.block_size+sector,buf);
  
	e2ino = & buf[offset_sector * cn_e2.s_inode_size];

}


get_current_dir(char *buf)
{
        int address;
	char *filename;
	int i=0,j;
        struct ext2_dir_entry_2 *e2dir;


	address = &buf[0];
	e2dir = &buf[0];
	while(e2dir->inode){
	        
	       
		filename = &e2dir->name[0];
		handler_current_dir(filename,e2dir->name_len,e2dir);
		
		if( (e2dir->name_len%4) ==0)
		  address = address +8 + e2dir->name_len;
	        else address = address +8 + (e2dir->name_len /4 +1 )*4;
		
		e2dir = address;
         }

}
static int dir_count=0;

handler_current_dir(char *filename,int name_len,struct ext2_dir_entry_2 *e2dir)
{
        char tmp;
	int i;
        tmp = filename[name_len];
	filename[name_len]='\0';
	current_dir[dir_count] = *e2dir;

	for(i = 0;i<name_len+1;i++)
	       current_dir[dir_count].name[i] = filename[i];

	dir_count ++;
       
       	filename[name_len] = tmp;
}
