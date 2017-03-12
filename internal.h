#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

#include <time.h>
#include <stdio.h>
#include <stdint.h>

/* Super block table.  The root file system and every mounted file system
 * has an entry here.  The entry holds information about the sizes of the bit
 * maps and inodes.  The s_ninodes field gives the number of inodes available
 * for files and directories, including the root directory.  Inode 0 is 
 * on the disk, but not used.  Thus s_ninodes = 4 means that 5 bits will be
 * used in the bit map, bit 0, which is always 1 and not used, and bits 1-4
 * for files and directories.  The disk layout is:
 *
 *      Item        # blocks
 *    boot block      1
 *    super block     1
 *    inode map     s_imap_blocks
 *    zone map      s_zmap_blocks
 *    inodes        (s_ninodes + 'inodes per block' - 1)/'inodes per block'
 *    unused        whatever is needed to fill out the current zone
 *    data zones    (s_zones - s_firstdatazone) << s_log_zone_size
 *
 * A super_block slot is free if s_dev == NO_DEV. 
 */

#define MINIX_BLOCK_SIZE 1024
#define BLOCK_SIZE MINIX_BLOCK_SIZE
#define usizeof sizeof

#define SUPER_V2_REV  0x2468	/* V2 magic written on PC, read on 68K or vv */

/* Tables sizes */
#define V1_NR_DZONES       7	/* # direct zone numbers in a V1 inode */
#define V1_NR_TZONES       9	/* total # zone numbers in a V1 inode */
#define V2_NR_DZONES       7	/* # direct zone numbers in a V2 inode */
#define V2_NR_TZONES      10	/* total # zone numbers in a V2 inode */

/* Types used in disk, inode, etc. data structures. */
typedef short          dev_t;	   /* holds (major|minor) device pair */
typedef char           gid_t;	   /* group id */
typedef unsigned short ino_t; 	   /* i-node number */
typedef unsigned short mode_t;	   /* file type and permissions bits */
typedef char         nlink_t;	   /* number of links to a file */
typedef unsigned long  off_t;	   /* offset within a file */
typedef int            pid_t;	   /* process id (must be signed) */
typedef short          uid_t;	   /* user id */
typedef unsigned long zone_t;	   /* zone number */
typedef unsigned long block_t;	   /* block number */
typedef unsigned long  bit_t;	   /* bit number in a bit map */
typedef unsigned short zone1_t;	   /* zone number for V1 file systems */
typedef unsigned short bitchunk_t; /* collection of bits in a bitmap */

typedef unsigned char   u8_t;	   /* 8 bit type */
typedef unsigned short u16_t;	   /* 16 bit type */
typedef unsigned long  u32_t;	   /* 32 bit type */

typedef char            i8_t;      /* 8 bit signed type */
typedef short          i16_t;      /* 16 bit signed type */
typedef long           i32_t;      /* 32 bit signed type */

#define ROOT_INODE         1	/* inode number for root directory */
#define BOOT_BLOCK  ((block_t) 0)	/* block number of boot block */
#define SUPER_BLOCK ((block_t) 1)	/* block number of super block */

#define DIR_ENTRY_SIZE       usizeof (struct direct)  /* # bytes/dir entry   */
#define NR_DIR_ENTRIES   (BLOCK_SIZE/DIR_ENTRY_SIZE)  /* # dir entries/blk   */
#define SUPER_SIZE      usizeof (struct super_block)  /* super_block size    */
#define PIPE_SIZE          (V1_NR_DZONES*BLOCK_SIZE)  /* pipe size in bytes  */
#define BITMAP_CHUNKS (BLOCK_SIZE/usizeof (bitchunk_t))/* # map chunks/blk   */

/* Derived sizes pertaining to the V1 file system. */
#define V1_ZONE_NUM_SIZE           usizeof (zone1_t)  /* # bytes in V1 zone  */
#define V1_INODE_SIZE             usizeof (d1_inode)  /* bytes in V1 dsk ino */
#define V1_INDIRECTS   (BLOCK_SIZE/V1_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V1_INODES_PER_BLOCK (BLOCK_SIZE/V1_INODE_SIZE)/* # V1 dsk inodes/blk */

/* Derived sizes pertaining to the V2 file system. */
#define V2_ZONE_NUM_SIZE            usizeof (zone_t)  /* # bytes in V2 zone  */
#define V2_INODE_SIZE             usizeof (d2_inode)  /* bytes in V2 dsk ino */
#define V2_INDIRECTS   (BLOCK_SIZE/V2_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V2_INODES_PER_BLOCK (BLOCK_SIZE/V2_INODE_SIZE)/* # V2 dsk inodes/blk */

struct super_block
{
  ino_t s_ninodes;		/* # usable inodes on the minor device */
  zone1_t  s_nzones;		/* total device size, including bit maps etc */
  short s_imap_blocks;		/* # of blocks used by inode bit map */
  short s_zmap_blocks;		/* # of blocks used by zone bit map */
  zone1_t s_firstdatazone;	/* number of first data zone */
  short s_log_zone_size;	/* log2 of blocks/zone */
  off_t s_max_size;		/* maximum file size on this device */
  short s_magic;		/* magic number to recognize super-blocks */
  short s_pad;			/* try to avoid compiler-dependent padding */
  zone_t s_zones;		/* number of zones (replaces s_nzones in V2) */
};

/* Declaration of the V1 inode as it is on the disk (not in core). */
typedef struct {		/* V1.x disk inode */
  mode_t d1_mode;		/* file type, protection, etc. */
  uid_t d1_uid;			/* user id of the file's owner */
  off_t d1_size;		/* current file size in bytes */
  time_t d1_mtime;		/* when was file data last changed */
  gid_t d1_gid;			/* group number */
  nlink_t d1_nlinks;		/* how many links to this file */
  u16_t d1_zone[V1_NR_TZONES];	/* block nums for direct, ind, and dbl ind */
} d1_inode;

/* Declaration of the V2 inode as it is on the disk (not in core). */
typedef struct {		/* V2.x disk inode */
  mode_t d2_mode;		/* file type, protection, etc. */
  u16_t d2_nlinks;		/* how many links to this file. HACK! */
  uid_t d2_uid;			/* user id of the file's owner. */
  u16_t d2_gid;			/* group number HACK! */
  off_t d2_size;		/* current file size in bytes */
  time_t d2_atime;		/* when was file data last accessed */
  time_t d2_mtime;		/* when was file data last changed */
  time_t d2_ctime;		/* when was inode data last changed */
  zone_t d2_zone[V2_NR_TZONES];	/* block nums for direct, ind, and dbl ind */
} d2_inode;

/* Flag bits for i_mode in the inode. */
#define I_TYPE          0170000	/* this field gives inode type */
#define I_REGULAR       0100000	/* regular file, not dir or special */
#define I_BLOCK_SPECIAL 0060000	/* block special file */
#define I_DIRECTORY     0040000	/* file is a directory */
#define I_CHAR_SPECIAL  0020000	/* character special file */
#define I_NAMED_PIPE	0010000 /* named pipe (FIFO) */
#define I_SET_UID_BIT   0004000	/* set effective uid_t on exec */
#define I_SET_GID_BIT   0002000	/* set effective gid_t on exec */
#define ALL_MODES       0006777	/* all bits for user, group and others */
#define RWX_MODES       0000777	/* mode bits for RWX only */
#define R_BIT           0000004	/* Rwx protection bit */
#define W_BIT           0000002	/* rWx protection bit */
#define X_BIT           0000001	/* rwX protection bit */
#define I_NOT_ALLOC     0000000	/* this inode is free */

#define MAX_NAME_LEN 14

typedef struct {		/* V2.x disk inode */
  u16_t inode;
  char name[MAX_NAME_LEN];
} minix_dir_entry;

typedef d2_inode minix_inode;

#endif // INTERNAL_H_INCLUDED
