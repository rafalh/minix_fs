#ifndef INODE_H_INCLUDED
#define INODE_H_INCLUDED

#include <windows.h>
#include "dokan.h"
#include "internal.h"

int MxfsReadInode(MINIX_FS *FileSys, int FileIdx, minix_inode *Result);
int MxfsWriteInode(MINIX_FS *FileSys, int FileIdx, minix_inode *Result);
int MxfsAllocZone(MINIX_FS *FileSys);
int MxfsGetBlockFromFileOffset(MINIX_FS *FileSys, unsigned FileIdx, minix_inode *FileInfo, unsigned Offset, BOOL Alloc);

int DOKAN_CALLBACK MxfsSetEndOfFile(
		LPCWSTR FileName,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo);

FORCEINLINE int MxfsZoneFromBit(MINIX_FS *FileSys, unsigned Bit)
{
    return FileSys->Super.s_firstdatazone + Bit - 1;
}

FORCEINLINE int MxfsBitFromZone(MINIX_FS *FileSys, unsigned Zone)
{
    return Zone - FileSys->Super.s_firstdatazone + 1;
}

#endif // INODE_H_INCLUDED
