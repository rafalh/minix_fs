#include "debug.h"
#include "general.h"
#include "file.h"
#include "bitmap.h"
#include "inode.h"

int MxfsReadInode(MINIX_FS *FileSys, int FileIdx, minix_inode *Result)
{
    if(FileIdx <= 0 || FileIdx > FileSys->Super.s_ninodes)
    {
        ERR("MxfsReadInode: Invalid inode %d\n", FileIdx);
        return -ERROR_INVALID_INDEX;
    }
    
    --FileIdx; // make it zero based
    unsigned InodesFirstBlock = SUPER_BLOCK + 1 + FileSys->Super.s_imap_blocks + FileSys->Super.s_zmap_blocks;
    unsigned Block = InodesFirstBlock + FileIdx * sizeof(minix_inode) / MINIX_BLOCK_SIZE;
    unsigned Offset = (FileIdx * sizeof(minix_inode)) % MINIX_BLOCK_SIZE;
    return MxfsCacheRead(FileSys, Result, Block, Offset, sizeof(minix_inode));
}

int MxfsWriteInode(MINIX_FS *FileSys, int FileIdx, minix_inode *Info)
{
    if(FileIdx <= 0 || FileIdx > FileSys->Super.s_ninodes)
    {
        ERR("MxfsWriteInode: Invalid inode %d\n", FileIdx);
        return -ERROR_INVALID_INDEX;
    }
    
    // Update inode modified time
    Info->d2_ctime = time(NULL);
    
    --FileIdx; // make it zero based
    unsigned InodesFirstBlock = SUPER_BLOCK + 1 + FileSys->Super.s_imap_blocks + FileSys->Super.s_zmap_blocks;
    unsigned Block = InodesFirstBlock + FileIdx * sizeof(minix_inode) / MINIX_BLOCK_SIZE;
    unsigned Offset = (FileIdx * sizeof(minix_inode)) % MINIX_BLOCK_SIZE;
    return MxfsCacheWrite(FileSys, Info, Block, Offset, sizeof(minix_inode));
}

int MxfsAllocZone(MINIX_FS *FileSys)
{
    int Bit = MxfsFindClearBit(&FileSys->ZoneMap);
    if(Bit < 0)
    {
        WARN("MxfsAllocZone: disk is full\n");
        return -ERROR_DISK_FULL;
    }
    
    unsigned Zone = MxfsZoneFromBit(FileSys, Bit);
    if(Zone > FileSys->Super.s_zones)
    {
        WARN("MxfsAllocZone: invalid zone %u > %lu\n", Zone, FileSys->Super.s_zones);
        return -ERROR_DISK_FULL;
    }
    
    int Result = MxfsSetBit(&FileSys->ZoneMap, Bit);
    ASSERT(Result >= 0);
    
    unsigned BitmapOffset = ALIGN_DOWN(Bit/8, MINIX_BLOCK_SIZE);
    unsigned BlockIdx = SUPER_BLOCK + 1 + FileSys->Super.s_imap_blocks + BitmapOffset/MINIX_BLOCK_SIZE;
    BYTE *Buffer = ((BYTE*)FileSys->ZoneMap.Buffer) + BitmapOffset;
    Result = MxfsCacheWrite(FileSys, Buffer, BlockIdx, 0, MINIX_BLOCK_SIZE);
    if(Result < 0)
    {
        WARN("MxfsAllocZone: MxfsCacheWrite failed\n");
        return Result;
    }
    
    WARN("Alloc zone %u\n", Zone);
    return Zone;
}

int MxfsFreeZone(MINIX_FS *FileSys, unsigned Zone)
{
    WARN("Free zone %u\n", Zone);
    
    if(Zone > FileSys->Super.s_zones)
    {
        WARN("MxfsFreeZone: invalid zone %u > %lu\n", Zone, FileSys->Super.s_zones);
        return -ERROR_INVALID_PARAMETER;
    }
    
    unsigned Bit = MxfsBitFromZone(FileSys, Zone);
    int Result = MxfsClearBit(&FileSys->ZoneMap, Bit);
    if(Result < 0)
    {
        WARN("MxfsFreeZone: MxfsClearBit failed\n");
        return Result;
    }
    
    unsigned BitmapOffset = ALIGN_DOWN(Bit/8, MINIX_BLOCK_SIZE);
    unsigned BlockIdx = SUPER_BLOCK + 1 + FileSys->Super.s_imap_blocks + BitmapOffset/MINIX_BLOCK_SIZE;
    BYTE *Buffer = ((BYTE*)FileSys->ZoneMap.Buffer) + BitmapOffset;
    return MxfsCacheWrite(FileSys, Buffer, BlockIdx, 0, MINIX_BLOCK_SIZE);
}

int MxfsGetBlockFromFileOffset(MINIX_FS *FileSys, unsigned FileIdx, minix_inode *FileInfo, unsigned Offset, BOOL Alloc)
{
    unsigned Block;
    zone_t Zone;
    unsigned OffsetInBlocks = Offset / MINIX_BLOCK_SIZE;
    unsigned OffsetInZones = OffsetInBlocks >> FileSys->Super.s_log_zone_size;
    unsigned FirstZoneBlockOffset = OffsetInZones << FileSys->Super.s_log_zone_size;
    
    if(OffsetInZones < V2_NR_DZONES)
    {
        // Use direct zone
        Zone = FileInfo->d2_zone[OffsetInZones];
        if(!Zone && Alloc)
        {
            Zone = MxfsAllocZone(FileSys);
            if(Zone)
            {
                FileInfo->d2_zone[OffsetInZones] = Zone;
                MxfsWriteInode(FileSys, FileIdx, FileInfo);
            }
        }
    }
    else
    {
        zone_t IndirectZone;
        unsigned IndirectBlock, IndIndex;
        int Result;
        
        IndIndex = OffsetInZones - V2_NR_DZONES;
        
        if(IndIndex < V2_INDIRECTS)
        {
            // Use single indirect zone
            IndirectZone = FileInfo->d2_zone[V2_NR_DZONES];
            if(!IndirectZone && Alloc)
            {
                IndirectZone = MxfsAllocZone(FileSys);
                if(IndirectZone)
                {
                    MxfsCacheZero(FileSys, IndirectZone << FileSys->Super.s_log_zone_size);
                    FileInfo->d2_zone[V2_NR_DZONES] = IndirectZone;
                    MxfsWriteInode(FileSys, FileIdx, FileInfo);
                }
            }
        }
        else // use double-indirect
        {
            zone_t DblIndZone;
            unsigned DblIndBlock, DblIndIndex;
            
            DblIndZone = FileInfo->d2_zone[V2_NR_DZONES + 1];
            if(!DblIndZone && Alloc)
            {
                DblIndZone = MxfsAllocZone(FileSys);
                if(DblIndZone)
                {
                    MxfsCacheZero(FileSys, DblIndZone << FileSys->Super.s_log_zone_size);
                    FileInfo->d2_zone[V2_NR_DZONES + 1] = DblIndZone;
                    MxfsWriteInode(FileSys, FileIdx, FileInfo);
                }
            }
            
            if(!DblIndZone)
                return -ERROR_NOT_FOUND;
            
            DblIndBlock = DblIndZone << FileSys->Super.s_log_zone_size;
            IndIndex -= V2_INDIRECTS;
            DblIndIndex = IndIndex / V2_INDIRECTS;
            IndIndex = IndIndex % V2_INDIRECTS;
            
            if(DblIndIndex > V2_INDIRECTS)
            {
                ERR("Too big offset!\n");
                return -ERROR_INVALID_PARAMETER;
            }
            
            Result = MxfsCacheRead(FileSys, &IndirectZone, DblIndBlock, DblIndIndex*sizeof(zone_t), sizeof(zone_t));
            if(Result < 0)
                return Result;
            
            if(!IndirectZone && Alloc)
            {
                IndirectZone = MxfsAllocZone(FileSys);
                if(IndirectZone)
                {
                    MxfsCacheZero(FileSys, IndirectZone << FileSys->Super.s_log_zone_size);
                    MxfsCacheWrite(FileSys, &IndirectZone, DblIndBlock, DblIndIndex*sizeof(zone_t), sizeof(zone_t));
                }
            }
        }
        
        if(!IndirectZone)
            return -ERROR_NOT_FOUND;
        
        IndirectBlock = IndirectZone << FileSys->Super.s_log_zone_size;
        Result = MxfsCacheRead(FileSys, &Zone, IndirectBlock, IndIndex*sizeof(zone_t), sizeof(zone_t));
        if(Result < 0)
            return Result;
        
        if(!Zone && Alloc)
        {
            Zone = MxfsAllocZone(FileSys);
            if(Zone)
            {
                MxfsCacheZero(FileSys, Zone << FileSys->Super.s_log_zone_size);
                MxfsCacheWrite(FileSys, &Zone, IndirectBlock, IndIndex*sizeof(zone_t), sizeof(zone_t));
            }
        }
    }
    
    if(!Zone)
        return -ERROR_NOT_FOUND;
    
    unsigned Bit = MxfsBitFromZone(FileSys, Zone);
    if(!MxfsGetBit(&FileSys->ZoneMap, Bit))
    {
        WARN("NOT GOOD! Bit %u is not set\n", Bit);
    }
    
    Block = Zone << FileSys->Super.s_log_zone_size;
    Block += OffsetInBlocks - FirstZoneBlockOffset; // add offset in zone
    return Block;
}

int DOKAN_CALLBACK MxfsSetEndOfFile(
		LPCWSTR FileName,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls", FileName);
    
    MINIX_FS *FileSys = (MINIX_FS*)(LONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(LONG_PTR)FileInfo->Context;
    ASSERT(FileCtx && Length >= 0);
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileCtx->Index, &Info);
    if(Result < 0)
        return Result;
    
    unsigned ZoneSize = MINIX_BLOCK_SIZE << FileSys->Super.s_log_zone_size;
    unsigned NewFirstFree = ALIGN_UP(Length, ZoneSize)/ZoneSize;
    unsigned FirstFree = ALIGN_UP(Info.d2_size, ZoneSize)/ZoneSize;
    
    if(NewFirstFree >= FirstFree)
        return 0; // done
    
    unsigned i = NewFirstFree;
    while(i < V2_NR_DZONES)
    {
        if(Info.d2_zone[i])
            MxfsFreeZone(FileSys, Info.d2_zone[i]);
        Info.d2_zone[i] = 0;
        ++i;
    }
    
    i -= V2_NR_DZONES;
    unsigned IndirZone = Info.d2_zone[V2_NR_DZONES];
    if(IndirZone && i < V2_INDIRECTS)
    {
        unsigned IndirBlock = IndirZone << FileSys->Super.s_log_zone_size;
        zone_t IndirTbl[V2_INDIRECTS];
        BOOL FreeIndirZone = (i == 0);
        
        Result = MxfsCacheRead(FileSys, IndirTbl, IndirBlock, 0, MINIX_BLOCK_SIZE);
        if(Result >= 0)
        {
            while(i < V2_INDIRECTS)
            {
                if(IndirTbl[i])
                    MxfsFreeZone(FileSys, IndirTbl[i]);
                IndirTbl[i] = 0;
                ++i;
            }
            if(FreeIndirZone)
                MxfsFreeZone(FileSys, IndirZone);
            else
                MxfsCacheWrite(FileSys, IndirTbl, IndirBlock, 0, MINIX_BLOCK_SIZE);
        } else
            ERR("Failed to read indirect block\n");
    }
    
    // TODO: Free dbl indirect block zones!
    Info.d2_size = Length;
    Info.d2_mtime = time(NULL);
    MxfsWriteInode(FileSys, FileCtx->Index, &Info);
    
    return 0;
}
