#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED

#include <stdio.h>
#include <windows.h>
#include "internal.h"
#include "disk_image.h"
#include "cache.h"
#include "bitmap.h"

#define ALIGN_UP(addr, n) (((addr) + (n) - 1)/(n)*(n))
#define ALIGN_DOWN(addr, n) ((addr)/(n)*(n))
#define COUNTOF(arr) (sizeof(arr)/sizeof((arr)[0]))

typedef struct _MINIX_FS
{
    DISK_IMG *pImg;
    unsigned uOffset, cbLen;
    struct super_block Super;
    MX_BITMAP InodeMap;
    MX_BITMAP ZoneMap;
    CRITICAL_SECTION Lock;
    MX_BLOCK_CACHE Cache;
} MINIX_FS;

MINIX_FS *MxfsOpen(DISK_IMG *pImg, unsigned uOffset, unsigned uLen);
void MxfsClose(MINIX_FS *pFileSys);

FORCEINLINE void *MxfsAlloc(unsigned uSize)
{
    return HeapAlloc(GetProcessHeap(), 0, uSize);
}

FORCEINLINE void MxfsFree(void *pMem)
{
    if(pMem)
        HeapFree(GetProcessHeap(), 0, pMem);
}

#define MINIX_VOLUME_ID 0x5F7AC349

#endif // GENERAL_H_INCLUDED
