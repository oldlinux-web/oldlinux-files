#include <string.h>

#include <cnix/ext2_fs.h>
#include <cnix/cfile.h>

extern struct cnix_ext2 cn_e2;
extern struct ext2_dir_entry_2 current_dir[20];


struct cfile *
cnix_open(char *dir,char *filename,unsigned char *file_buf)
{
       int i=0;
       struct ext2_inode e2ino;
       unsigned char buf[512];
       struct cfile *file;
     
       file = cnix_malloc(8);

       while(cn_e2.root_dir[i].inode)
       {
	    if(strncmp(cn_e2.root_dir[i].name,dir,strlen(dir))==0)
	      break;
	    i++;
       }
       
       read_inode(cn_e2.root_dir[i].inode,buf,&e2ino);

       lba_read_sector(cn_e2.start_sect +e2ino.i_block[0] * cn_e2.block_size,buf);
       get_current_dir(&buf);

       i = 0;
       while(current_dir[i].inode)
       {
	    if(strncmp(current_dir[i].name,filename,strlen(filename))==0)
	      break;
	    i++;
       }

       read_inode(current_dir[i].inode,buf,&e2ino);


       {
	     int i,j,file_block;
	     unsigned long file_long[256];
	     
	     file_block = e2ino.i_size/512/cn_e2.block_size +1;
	     file_buf = cnix_malloc(file_block * cn_e2.block_size *512);

	     if (file_block <12) j = file_block;
	     else j = 12;
	     /*the 12 block*/
	     for(i=0;i<j*cn_e2.block_size;i++)
	       {
		 lba_read_sector(cn_e2.start_sect +e2ino.i_block[i/cn_e2.block_size] * cn_e2.block_size+i%cn_e2.block_size,file_buf+i*512);
	       }

	     /*for 2nd block*/
	     /*get the 2nd block bitmap table*/
	     if(file_block >12){
	      lba_read_sector(cn_e2.start_sect +e2ino.i_block[12] *cn_e2.block_size,file_long); 
	      lba_read_sector(cn_e2.start_sect +e2ino.i_block[12] *cn_e2.block_size+1,&file_long[128]);
	     }
	     /*read the 2nd data*/
	     for(i=0;i<(file_block-12);i++)
	       {/*per block read*/
		 int off_j;
		 for(off_j=0;off_j<cn_e2.block_size;off_j++)
		 {/*per sector read*/
		   lba_read_sector(cn_e2.start_sect + file_long[i]*cn_e2.block_size+off_j,file_buf+12*512+i*cn_e2.block_size+off_j*512);
		 }

	       }
	     

       }
       file->size = e2ino.i_size;
       file->file_buf = file_buf;

       return file;
#if 0
       printk("filename:%s\n",filename);
       printk(buf);
       printk("\n");
#endif
}
print_hex(unsigned char *buf)
{
	 int j=0;
	 for(j=0;j<512;j++)
	   printk("%02x ",buf[j]);
       printk("\n");       
}
