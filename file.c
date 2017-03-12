#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "file.h"
#include "dir.h"
#include "debug.h"
#include "internal.h"
#include "inode.h"

int DOKAN_CALLBACK MxfsCreateFile(
		LPCWSTR FileName,
		DWORD DesiredAccess,
		DWORD ShareMode,
		DWORD CreationDisposition,
		DWORD FlagsAndAttributes,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("FileName %ls DesiredAccess %lx CreationDisposition %ld FileInfo %p\n", FileName, DesiredAccess, CreationDisposition, FileInfo);
    
    char Path[MAX_PATH];
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    
    wcstombs(Path, FileName, sizeof(Path));
    int FileIdx = MxfsParsePath(FileSys, Path, FALSE);
    if(FileIdx < 0)
    {
        INFO("Failed to create file %s\n", Path);
        return -ERROR_FILE_NOT_FOUND;
    }
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileIdx, &Info);
    if(Result < 0)
        return Result;
    
    if(CreationDisposition == CREATE_NEW)
    {
        INFO("Already exists %s\n", Path);
        return -ERROR_ALREADY_EXISTS;
    }
    
    FILE_CTX *FileCtx = MxfsAlloc(sizeof(FILE_CTX));
    if(!FileCtx)
        return -ERROR_NOT_ENOUGH_MEMORY;
    
    FileCtx->Index = FileIdx;
    FileCtx->Write = (DesiredAccess & (GENERIC_WRITE|GENERIC_ALL|FILE_WRITE_DATA)) ? TRUE : FALSE;
    FileCtx->Changed = FALSE;
#ifndef NDEBUG
    wcscpy_s(FileCtx->DbgBuf, sizeof(FileCtx->DbgBuf), FileName);
#endif // NDEBUG
    
    if(FileInfo->Context)
        WARN("MxfsCreateFile: Context %I64u\n", FileInfo->Context);
    FileInfo->Context = (LONG64)(ULONG_PTR)FileCtx;
    FileInfo->IsDirectory = (Info.d2_mode & I_DIRECTORY) ? TRUE : FALSE;
    
    FileCtx->Length = Info.d2_size;
    if(CreationDisposition == TRUNCATE_EXISTING || CreationDisposition == CREATE_ALWAYS)
        FileCtx->Length = 0;
    if(FileCtx->Write)
        INFO("CreationDisposition %ld\n", CreationDisposition);
    
    return 0;
}

int DOKAN_CALLBACK MxfsOpenDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", FileName, FileInfo);
    
    char Path[MAX_PATH];
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    
    wcstombs(Path, FileName, sizeof(Path));
    int FileIdx = MxfsParsePath(FileSys, Path, FALSE);
    if(FileIdx < 0)
    {
        INFO("Failed to open dir %s\n", Path);
        return -ERROR_FILE_NOT_FOUND;
    }
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileIdx, &Info);
    if(Result < 0)
        return Result;
    
    FILE_CTX *FileCtx = MxfsAlloc(sizeof(FILE_CTX));
    if(!FileCtx)
        return -ERROR_NOT_ENOUGH_MEMORY;
    
    FileCtx->Index = FileIdx;
    FileCtx->Length = Info.d2_size;
    FileCtx->Changed = FALSE;
#ifndef NDEBUG
    wcscpy_s(FileCtx->DbgBuf, sizeof(FileCtx->DbgBuf), FileName);
#endif // NDEBUG
    
    if(!(Info.d2_mode & I_DIRECTORY))
    {
        MxfsFree(FileCtx);
        INFO("Failed to open dir %s\n", Path);
        return -ERROR_INVALID_PARAMETER;
    }
    
    if(FileInfo->Context)
        WARN("MxfsOpenDirectory: Context %I64u\n", FileInfo->Context);
    
    FileInfo->IsDirectory = TRUE;
    FileInfo->Context = (ULONG64)(ULONG_PTR)FileCtx;
    
    return 0;
}

int DOKAN_CALLBACK MxfsCreateDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", FileName, FileInfo);
    
    char Path[MAX_PATH];
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    
    wcstombs(Path, FileName, sizeof(Path));
    int FileIdx = MxfsParsePath(FileSys, Path, FALSE);
    if(FileIdx > 0)
        return -ERROR_ALREADY_EXISTS;
    
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

// When FileInfo->DeleteOnClose is true, you must delete the file in Cleanup.
int DOKAN_CALLBACK MxfsCleanup(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", FileName, FileInfo);
    
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(ULONG_PTR)FileInfo->Context;
    ASSERT(FileCtx);
    
    minix_inode Info;
    if(MxfsReadInode(FileSys, FileCtx->Index, &Info) >= 0)
    {
        Info.d2_atime = time(NULL);
        
        if(FileCtx->Changed)
        {
            Info.d2_mtime = time(NULL);
            Info.d2_ctime = time(NULL);
        }
        
        MxfsWriteInode(FileSys, FileCtx->Index, &Info);
    }
    
    if(FileCtx->Changed)
        MxfsSetEndOfFile(FileName, FileCtx->Length, FileInfo);
    
    MxfsFree(FileCtx);
    FileInfo->Context = 0;
    
    return 0;
}

int DOKAN_CALLBACK MxfsCloseFile(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", FileName, FileInfo);
    
    ASSERT(FileInfo->Context == 0);
    
    return 0;
}
