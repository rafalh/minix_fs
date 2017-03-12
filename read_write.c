#include <stdlib.h>
#include "debug.h"
#include "file.h"
#include "read_write.h"
#include "inode.h"

int MxfsReadBlocks(MINIX_FS *FileSys, void *Buffer, unsigned FirstBlock, unsigned BlocksCount)
{
    while(BlocksCount > 0)
    {
        int Result = MxfsCacheRead(FileSys, Buffer, FirstBlock, 0, MINIX_BLOCK_SIZE);
        if(Result < 0)
            return Result;
        
        Buffer = ((PBYTE)Buffer) + MINIX_BLOCK_SIZE;
        ++FirstBlock;
        --BlocksCount;
    }
    return 0;
}

int DOKAN_CALLBACK MxfsReadFile(
		LPCWSTR FileName,
		LPVOID Buffer,
		DWORD NumberOfBytesToRead,
		LPDWORD NumberOfBytesRead,
		LONGLONG Offset,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %lu %I64u", FileName, NumberOfBytesToRead, Offset);
    
    MINIX_FS *FileSys = (MINIX_FS*)(LONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(LONG_PTR)FileInfo->Context;
    if(!FileCtx || Offset < 0)
    {
        ERR("Invalid params %p %I64d\n", FileCtx, Offset);
        return -ERROR_INVALID_PARAMETER;
    }
    ASSERT(FileCtx && Offset >= 0);
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileCtx->Index, &Info);
    if(Result < 0)
        return Result;
    
    *NumberOfBytesRead = 0;
    
    if(Offset > FileCtx->Length)
        Offset = FileCtx->Length;
    if(Offset + NumberOfBytesToRead > FileCtx->Length)
        NumberOfBytesToRead = FileCtx->Length - Offset;
    
    while(NumberOfBytesToRead > 0)
    {
        int Block = MxfsGetBlockFromFileOffset(FileSys, FileCtx->Index, &Info, Offset, FALSE);
        if(Block < 0)
            return Block;
        
        unsigned ChunkOffset = Offset % MINIX_BLOCK_SIZE;
        unsigned ChunkLength = min(NumberOfBytesToRead, MINIX_BLOCK_SIZE - ChunkOffset);
        
        int Result = MxfsCacheRead(FileSys, Buffer, Block, ChunkOffset, ChunkLength);
        if(Result < 0)
            return Result;
        
        Buffer = ((PBYTE)Buffer) + ChunkLength;
        Offset += ChunkLength;
        NumberOfBytesToRead -= ChunkLength;
        *NumberOfBytesRead += ChunkLength;
    }
    
    return 0;
}

int DOKAN_CALLBACK MxfsWriteFile(
		LPCWSTR FileName,
		LPCVOID Buffer,
		DWORD NumberOfBytesToWrite,
		LPDWORD NumberOfBytesWritten,
		LONGLONG Offset,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %lu %I64d\n", FileName, NumberOfBytesToWrite, Offset);
    
    MINIX_FS *FileSys = (MINIX_FS*)(LONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(LONG_PTR)FileInfo->Context;
    ASSERT(FileCtx && Offset >= 0);
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileCtx->Index, &Info);
    if(Result < 0)
        return Result;
    
    *NumberOfBytesWritten = 0;
    
    if(Offset > FileCtx->Length)
        Offset = FileCtx->Length;
    
    while(NumberOfBytesToWrite > 0)
    {
        int Block = MxfsGetBlockFromFileOffset(FileSys, FileCtx->Index, &Info, Offset, TRUE);
        if(Block < 0)
        {
            ERR("MxfsGetBlockFromFileOffset failed %I64d\n", Offset);
            Result = Block;
            break;
        }
        
        unsigned ChunkOffset = Offset % MINIX_BLOCK_SIZE;
        unsigned ChunkLength = min(NumberOfBytesToWrite, MINIX_BLOCK_SIZE - ChunkOffset);
        
        Result = MxfsCacheWrite(FileSys, Buffer, Block, ChunkOffset, ChunkLength);
        if(Result < 0)
            break;
        
        Buffer = ((PBYTE)Buffer) + ChunkLength;
        Offset += ChunkLength;
        NumberOfBytesToWrite -= ChunkLength;
        *NumberOfBytesWritten += ChunkLength;
    }
    
    if(*NumberOfBytesWritten > 0)
    {
        if(Offset > FileCtx->Length)
            FileCtx->Length = Offset;
        
        FileCtx->Changed = TRUE;
    }
    
    return Result;
}
