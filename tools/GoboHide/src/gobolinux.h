#ifndef _LINUX_GOBOLINUX_H
#define _LINUX_GOBOLINUX_H

/* Gobolinux internal ioctls */

#define GOBOLINUX_HIDEINODE   0x0000001 /* Hide a given inode number */
#define GOBOLINUX_UNHIDEINODE 0x0000002 /* Unhide a given inode number */
#define GETSTATSUIDNUMBER     0x0000003 /* Get the _number_ of inodes 
					   being hidden */
#define GETSTATSUID           0x0000004 /* Get the inodes hidden */

struct gobolinux_hide_stats {
	int hidden_inodes;      /* how many inodes we're hiding */
	int filled_size;        /* how many inodes we filled on the hidden_list */
	char **hidden_list;     /* the hidden list */
};

struct gobolinux_hide {
	ino_t inode;                        /* the inode number */
	const char *pathname;               /* the pathname being submitted */
	char symlink;                       /* is inode a symlink? */
	char operation;                     /* the operation to be performed */
	struct gobolinux_hide_stats stats;  /* statistics about the inodes being hidden */
};

#ifdef __KERNEL__

struct dentry *gobolinux_get_dentry (ino_t ino, const char *pathname);
int  gobolinux_count_hidden (struct gobolinux_hide *hide);
int  gobolinux_show_hidden (struct gobolinux_hide *hide);
int  gobolinux_hide (ino_t ino); 
int  gobolinux_inode_add (ino_t ino, const char *pathname);
int  gobolinux_inode_del (ino_t ino);
int  gobolinux_fs_ioctl (struct inode *inode, struct gobolinux_hide *hide);

#endif /* __KERNEL__ */

#endif  /* _LINUX_GOBOLINUX_H */
