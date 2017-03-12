#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <windows.h>
#include "internal.h"
#include "list.h"

#define BLOCK_CACHE_SIZE 128

struct _MINIX_FS;

typedef struct _MX_CACHE_ITEM
{
    LIST_ENTRY MruList;
    unsigned Index;
    BOOL Dirty;
    BYTE Data[MINIX_BLOCK_SIZE];
} MX_CACHE_ITEM;

typedef struct
{
    LIST_ENTRY MruList;
    unsigned Count;
    CRITICAL_SECTION Lock;
    struct _MINIX_FS *FileSys;
} MX_BLOCK_CACHE;

void MxfsCacheInit(struct _MINIX_FS *FileSys);
void MxfsCacheDestroy(struct _MINIX_FS *FileSys);
void MxfsCacheFlush(struct _MINIX_FS *FileSys);
int MxfsCacheRead(struct _MINIX_FS *FileSys, void *Buffer, unsigned Block, unsigned Offset, unsigned Length);
int MxfsCacheWrite(struct _MINIX_FS *FileSys, const void *Buffer, unsigned Block, unsigned Offset, unsigned Length);
void MxfsCacheZero(struct _MINIX_FS *FileSys, unsigned Block);

#endif // CACHE_H_INCLUDED
