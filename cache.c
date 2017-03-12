#include <stdlib.h>
#include "debug.h"
#include "cache.h"
#include "general.h"

void MxfsCacheInit(MINIX_FS *FileSys)
{
    InitializeListHead(&FileSys->Cache.MruList);
    FileSys->Cache.Count = 0;
    InitializeCriticalSection(&FileSys->Cache.Lock);
}

static int MxfsCacheFlushItem(MINIX_FS *FileSys, MX_CACHE_ITEM *Item)
{
    unsigned Offset = FileSys->uOffset + Item->Index*MINIX_BLOCK_SIZE;
    int Result = ImgWrite(FileSys->pImg, Offset, MINIX_BLOCK_SIZE, Item->Data);
    Item->Dirty = FALSE;
    
    if(Result < 0)
        ERR("MxfsCacheFlushItem: ImgWrite failed %d\n", Result);
    return Result;
}

void MxfsCacheFlush(MINIX_FS *FileSys)
{
    EnterCriticalSection(&FileSys->Cache.Lock);
    
    LIST_ENTRY *Entry = FileSys->Cache.MruList.Flink;
    while(Entry != &FileSys->Cache.MruList)
    {
        MX_CACHE_ITEM *Item = CONTAINING_RECORD(Entry, MX_CACHE_ITEM, MruList);
        if(Item->Dirty)
            MxfsCacheFlushItem(FileSys, Item);
        
        Entry = Entry->Flink;
    }
    
    LeaveCriticalSection(&FileSys->Cache.Lock);
}

static void MxfsCacheDestroyLastItem(MINIX_FS *FileSys)
{
    ASSERT(FileSys->Cache.Count > 0);
    
    LIST_ENTRY *Entry = RemoveTailList(&FileSys->Cache.MruList);
    MX_CACHE_ITEM *Item = CONTAINING_RECORD(Entry, MX_CACHE_ITEM, MruList);
    --FileSys->Cache.Count;
    
    if(Item->Dirty)
        MxfsCacheFlushItem(FileSys, Item);
    
    MxfsFree(Item);
}

void MxfsCacheDestroy(MINIX_FS *FileSys)
{
    while(!IsListEmpty(&FileSys->Cache.MruList))
        MxfsCacheDestroyLastItem(FileSys);
}

MX_CACHE_ITEM *MxfsCacheFind(MINIX_FS *FileSys, unsigned Block)
{
    LIST_ENTRY *Entry = FileSys->Cache.MruList.Flink;
    while(Entry != &FileSys->Cache.MruList)
    {
        MX_CACHE_ITEM *Item = CONTAINING_RECORD(Entry, MX_CACHE_ITEM, MruList);
        if(Item->Index == Block)
            return Item;
        
        Entry = Entry->Flink;
    }
    return NULL;
}

MX_CACHE_ITEM *MxfsCacheLoadBlock(MINIX_FS *FileSys, unsigned Block, BOOL Zero)
{
    MX_CACHE_ITEM *Item = MxfsAlloc(sizeof(MX_CACHE_ITEM));
    if(!Item) return NULL;
    
    unsigned BlockOffset = Block * MINIX_BLOCK_SIZE;
    if(BlockOffset + MINIX_BLOCK_SIZE > FileSys->cbLen)
    {
        ERR("MxfsCacheLoadBlock: Invalid block %u\n", Block);
        return NULL;
    }
    
    Item->Index = Block;
    
    if(!Zero)
    {
        int Result = ImgRead(FileSys->pImg, FileSys->uOffset + BlockOffset, MINIX_BLOCK_SIZE, Item->Data);
        if(Result < 0)
        {
            MxfsFree(Item);
            ERR("MxfsCacheLoadBlock: ImgRead failed %d\n", Result);
            return NULL;
        }
        Item->Dirty = FALSE;
    } else {
        memset(Item->Data, 0, MINIX_BLOCK_SIZE);
        Item->Dirty = TRUE;
    }
    
    if(FileSys->Cache.Count == BLOCK_CACHE_SIZE)
        MxfsCacheDestroyLastItem(FileSys);
    
    InsertHeadList(&FileSys->Cache.MruList, &Item->MruList);
    ++FileSys->Cache.Count;
    
    return Item;
}

int MxfsCacheRead(MINIX_FS *FileSys, void *Buffer, unsigned Block, unsigned Offset, unsigned Length)
{
    if(!(Offset < MINIX_BLOCK_SIZE && Offset + Length <= MINIX_BLOCK_SIZE))
    {
        WARN("MxfsCacheRead: wrong offset (%u) or len (%u)\n", Offset, Length);
        return -ERROR_INVALID_PARAMETER;
    }
    
    EnterCriticalSection(&FileSys->Cache.Lock);
    
    int Result = -ERROR_NOT_FOUND;
    MX_CACHE_ITEM *Item = MxfsCacheFind(FileSys, Block);
    if(Item)
    {
        // Push at front
        RemoveEntryList(&Item->MruList);
        InsertHeadList(&FileSys->Cache.MruList, &Item->MruList);
    } else
        Item = MxfsCacheLoadBlock(FileSys, Block, FALSE);
    
    if(Item)
    {
        memcpy(Buffer, Item->Data + Offset, Length);
        Result = 0;
    }
    
    LeaveCriticalSection(&FileSys->Cache.Lock);
    return Result;
}

int MxfsCacheWrite(MINIX_FS *FileSys, const void *Buffer, unsigned Block, unsigned Offset, unsigned Length)
{
    if(!(Offset < MINIX_BLOCK_SIZE && Offset + Length <= MINIX_BLOCK_SIZE))
    {
        WARN("MxfsCacheWrite: wrong offset (%u) or len (%u)\n", Offset, Length);
        return -ERROR_INVALID_PARAMETER;
    }
    
    EnterCriticalSection(&FileSys->Cache.Lock);
    
    int Result = -ERROR_WRITE_FAULT;
    MX_CACHE_ITEM *Item = MxfsCacheFind(FileSys, Block);
    if(!Item)
        Item = MxfsCacheLoadBlock(FileSys, Block, FALSE);
    
    if(Item)
    {
        memcpy(Item->Data + Offset, Buffer, Length);
        Item->Dirty = TRUE;
        Result = 0;
    }
    
    LeaveCriticalSection(&FileSys->Cache.Lock);
    return Result;
}

void MxfsCacheZero(MINIX_FS *FileSys, unsigned Block)
{
    EnterCriticalSection(&FileSys->Cache.Lock);
    
    MX_CACHE_ITEM *Item = MxfsCacheFind(FileSys, Block);
    if(Item)
    {
        memset(Item->Data, 0, MINIX_BLOCK_SIZE);
        Item->Dirty = TRUE;
    }
    else
        MxfsCacheLoadBlock(FileSys, Block, TRUE);
    
    LeaveCriticalSection(&FileSys->Cache.Lock);
}

