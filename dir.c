#include <stdlib.h>
#include <string.h>
#include "dir.h"
#include "general.h"
#include "debug.h"
#include "internal.h"
#include "file.h"
#include "inode.h"
#include "file_info.h"

int MxfsFindFileInDir(MINIX_FS *FileSys, unsigned Inode, minix_inode *DirInfo, const char *Filename)
{
    unsigned TotalEntries, i, j = 0, Offset;
    int Result;
    minix_dir_entry Entries[MINIX_BLOCK_SIZE / sizeof(minix_dir_entry)];
    
    TotalEntries = DirInfo->d2_size / sizeof(minix_dir_entry);
    
    for(i = 0; i < TotalEntries; ++i)
    {
        if(i % COUNTOF(Entries) == 0)
        {
            Offset = i * sizeof(minix_dir_entry);
            Result = MxfsGetBlockFromFileOffset(FileSys, Inode, DirInfo, Offset, FALSE);
            if(Result < 0)
                return Result;
            
            Result = MxfsCacheRead(FileSys, Entries, Result, 0, MINIX_BLOCK_SIZE);
            if(Result < 0)
                return Result;
            
            j = 0;
        }
        
        if(Entries[j].inode != 0 && strncmp(Filename, Entries[j].name, MAX_NAME_LEN) == 0)
            return Entries[j].inode;
        
        ++j;
    }
    return -1;
}

int MxfsParsePath(MINIX_FS *FileSys, const char *Path, BOOL Parent)
{
    char Filename[32];
    unsigned FilenameLen = 0;
    int Result = 1;
    minix_inode DirInfo;
    
    while(1)
    {
        if(*Path != '/' && *Path != '\\' && *Path != '\0')
        {
            if(FilenameLen + 1 >= sizeof(Filename))
                return -ERROR_NOT_FOUND;
            Filename[FilenameLen++] = *Path;
        }
        else if(FilenameLen != 0)
        {
            if(Parent && *Path == '\0')
                break;
            
            Filename[FilenameLen] = '\0';
            MxfsReadInode(FileSys, Result, &DirInfo);
            
            if(!(DirInfo.d2_mode & I_DIRECTORY))
                return -ERROR_NOT_FOUND;
            
            Result = MxfsFindFileInDir(FileSys, Result, &DirInfo, Filename);
            FilenameLen = 0;
        }
        
        if(Result < 0 || *Path == '\0')
            break;
        
        ++Path;
    }
    
    return Result;
}

int DOKAN_CALLBACK MxfsFindFiles(
		LPCWSTR PathName,
		PFillFindData Callback,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", PathName, FileInfo);
    ASSERT(FileInfo->IsDirectory);
    
    int Result;
    minix_dir_entry Entries[MINIX_BLOCK_SIZE / sizeof(minix_dir_entry)];
    unsigned TotalEntries, FileIdx, Offset, i, j;
    minix_inode DirInfo, Info;
    
    MINIX_FS *FileSys = (MINIX_FS*)(LONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(LONG_PTR)FileInfo->Context;
    ASSERT(FileCtx);
    
    Result = MxfsReadInode(FileSys, FileCtx->Index, &DirInfo);
    if(Result < 0)
        return Result;
    
    TotalEntries = DirInfo.d2_size / sizeof(minix_dir_entry);
    if(TotalEntries == 0)
        return 0; // empty dir
    
    WIN32_FIND_DATAW wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    
    for(i = 0, j = 0; i < TotalEntries; ++i, ++j)
    {
        if(i % COUNTOF(Entries) == 0)
        {
            Offset = i * sizeof(minix_dir_entry);
            Result = MxfsGetBlockFromFileOffset(FileSys, FileCtx->Index, &DirInfo, Offset, FALSE);
            if(Result < 0)
                return Result;
            
            Result = MxfsCacheRead(FileSys, Entries, Result, 0, MINIX_BLOCK_SIZE);
            if(Result < 0)
                return Result;
            
            j = 0;
        }
        
        FileIdx = Entries[j].inode;
        if(!FileIdx)
            continue;
        
        if(MxfsReadInode(FileSys, FileIdx, &Info) < 0)
        {
            WARN("Invalid node in directory\n");
            continue;
        }
        
        MxfsFillFindData(&wfd, &Entries[j], &Info);
        Callback(&wfd, FileInfo);
    }
    
    return 0;
}
