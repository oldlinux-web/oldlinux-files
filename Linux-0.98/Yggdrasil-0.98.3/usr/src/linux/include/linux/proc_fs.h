#ifndef _LINUX_PROC_FS_H
#define _LINUX_PROC_FS_H

/*
 * The proc filesystem constants/structures
 */

#define PROC_ROOT_INO 1

#define PROC_SUPER_MAGIC 0x9fa0

extern struct super_block *proc_read_super(struct super_block *,void *);
extern void proc_put_inode(struct inode *);
extern void proc_put_super(struct super_block *);
extern void proc_statfs(struct super_block *, struct statfs *);
extern void proc_read_inode(struct inode *);
extern void proc_write_inode(struct inode *);

extern struct inode_operations proc_root_inode_operations;
extern struct inode_operations proc_base_inode_operations;
extern struct inode_operations proc_mem_inode_operations;
extern struct inode_operations proc_link_inode_operations;
extern struct inode_operations proc_fd_inode_operations;

#endif
