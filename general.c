#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "general.h"
#include "internal.h"
#include "debug.h"
#include "read_write.h"

MINIX_FS *MxfsOpen(DISK_IMG *pImg, unsigned uOffset, unsigned cbLen)
{
    MINIX_FS *FileSys;
    
    TRACE("");
    assert(sizeof(d2_inode) == 64);
    
    FileSys = MxfsAlloc(sizeof(*FileSys));
    if(!FileSys)
        return NULL;
    
    memset(FileSys, 0, sizeof(*FileSys));
    FileSys->pImg = pImg;
    FileSys->uOffset = uOffset;
    FileSys->cbLen = cbLen;
    InitializeCriticalSection(&FileSys->Lock);
    MxfsCacheInit(FileSys);
    
    if(MxfsCacheRead(FileSys, &FileSys->Super, SUPER_BLOCK, 0, sizeof(FileSys->Super)) != 0)
    {
        MxfsClose(FileSys);
        return NULL;
    }
    
    if(FileSys->Super.s_magic != SUPER_V2_REV)
    {
        ERR("Invalid magic in super block %x\n", FileSys->Super.s_magic);
        MxfsClose(FileSys);
        return NULL;
    }
    
    unsigned InodeMapSize = FileSys->Super.s_imap_blocks * MINIX_BLOCK_SIZE;
    uint32_t *InodeMapBuf = MxfsAlloc(InodeMapSize);
    if(!InodeMapBuf || MxfsReadBlocks(FileSys, InodeMapBuf, SUPER_BLOCK + 1, FileSys->Super.s_imap_blocks) != 0)
    {
        MxfsFree(InodeMapBuf);
        MxfsClose(FileSys);
        return NULL;
    }
    MxfsInitBitmap(&FileSys->InodeMap, InodeMapBuf, InodeMapSize * 8);
    
    unsigned ZoneMapBlock = SUPER_BLOCK + 1 + FileSys->Super.s_imap_blocks;
    unsigned ZoneMapSize = FileSys->Super.s_zmap_blocks * MINIX_BLOCK_SIZE;
    uint32_t *ZoneMapBuf = MxfsAlloc(ZoneMapSize);
    if(!ZoneMapBuf || MxfsReadBlocks(FileSys, ZoneMapBuf, ZoneMapBlock, FileSys->Super.s_zmap_blocks) != 0)
    {
        MxfsFree(ZoneMapBuf);
        MxfsClose(FileSys);
        return NULL;
    }
    MxfsInitBitmap(&FileSys->ZoneMap, ZoneMapBuf, ZoneMapSize * 8);
    
    return FileSys;
}

void MxfsClose(MINIX_FS *FileSys)
{
    TRACE("");
    if(!FileSys) return;
    MxfsCacheDestroy(FileSys); // destroy cache first (flush)
    MxfsDestroyBitmap(&FileSys->InodeMap);
    MxfsDestroyBitmap(&FileSys->ZoneMap);
    memset(FileSys, 0, sizeof(*FileSys));
    MxfsFree(FileSys);
}
