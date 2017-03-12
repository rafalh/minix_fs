#include "file_info.h"
#include "file.h"
#include "general.h"
#include "debug.h"
#include "inode.h"

void MxfsFileTimeFromTimeT(LPFILETIME FileTime, time_t t)
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000ull;
    FileTime->dwLowDateTime = (DWORD)ll;
    FileTime->dwHighDateTime = ll >> 32;
}

time_t MxfsTimeTFromFileTime(CONST FILETIME *FileTime)
{
   ULARGE_INTEGER ull;
   ull.LowPart = FileTime->dwLowDateTime;
   ull.HighPart = FileTime->dwHighDateTime;
   return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

int DOKAN_CALLBACK MxfsGetFileInformation(
		LPCWSTR FileName,
		LPBY_HANDLE_FILE_INFORMATION Buffer,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls %p", FileName, FileInfo);
    
    MINIX_FS *FileSys = (MINIX_FS*)(LONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(LONG_PTR)FileInfo->Context;
    ASSERT(FileCtx);
    
    minix_inode Info;
    MxfsReadInode(FileSys, FileCtx->Index, &Info);
    
    ZeroMemory(Buffer, sizeof(*Buffer));
    Buffer->dwFileAttributes = FileInfo->IsDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    MxfsFileTimeFromTimeT(&Buffer->ftCreationTime, Info.d2_mtime);
    MxfsFileTimeFromTimeT(&Buffer->ftLastAccessTime, Info.d2_atime);
    MxfsFileTimeFromTimeT(&Buffer->ftLastWriteTime, Info.d2_mtime);
    Buffer->dwVolumeSerialNumber = MINIX_VOLUME_ID;
    Buffer->nFileSizeLow = Info.d2_size;
    Buffer->nNumberOfLinks = 1;
    Buffer->nFileIndexLow = FileCtx->Index;
    
    return 0;
}

int DOKAN_CALLBACK MxfsSetFileTime(
		LPCWSTR FileName,
		CONST FILETIME *CreationTime,
		CONST FILETIME *LastAccessTime,
		CONST FILETIME *LastWriteTime,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls", FileName);
    
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    FILE_CTX *FileCtx = (FILE_CTX*)(ULONG_PTR)FileInfo->Context;
    ASSERT(FileCtx);
    
    minix_inode Info;
    int Result = MxfsReadInode(FileSys, FileCtx->Index, &Info);
    if(Result < 0)
        return Result;
    
    if(LastAccessTime && (LastAccessTime->dwHighDateTime || LastAccessTime->dwLowDateTime))
    {
        if(LastAccessTime->dwHighDateTime == 0xFFFFFFFF && LastAccessTime->dwLowDateTime == 0xFFFFFFFF)
            WARN("TODO: Support -1!\n"); // preserve access time
        else
            Info.d2_atime = MxfsTimeTFromFileTime(LastAccessTime);
    }
    
    if(LastWriteTime && (LastWriteTime->dwHighDateTime || LastWriteTime->dwLowDateTime))
    {
        if(LastWriteTime->dwHighDateTime == 0xFFFFFFFF && LastWriteTime->dwLowDateTime == 0xFFFFFFFF)
            WARN("TODO: Support -1!\n"); // preserve write time
        else
            Info.d2_mtime = MxfsTimeTFromFileTime(LastWriteTime);
    }
    
    return MxfsWriteInode(FileSys, FileCtx->Index, &Info);
}

void MxfsFillFindData(WIN32_FIND_DATAW *pFindData, const minix_dir_entry *pDirEntry, const minix_inode *pInfo)
{
    /* Attributes */
    if(pInfo->d2_mode & I_DIRECTORY)
        pFindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    else
        pFindData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    
    /* Times */
    MxfsFileTimeFromTimeT(&pFindData->ftCreationTime, pInfo->d2_mtime);
    MxfsFileTimeFromTimeT(&pFindData->ftLastAccessTime, pInfo->d2_atime);
    MxfsFileTimeFromTimeT(&pFindData->ftLastWriteTime, pInfo->d2_mtime);
    
    /* Size */
    pFindData->nFileSizeLow = pInfo->d2_size;
    
    /* Name */
    int NameLen = strnlen(pDirEntry->name, MAX_NAME_LEN);
    MultiByteToWideChar(CP_ACP, 0, pDirEntry->name, NameLen, pFindData->cFileName, COUNTOF(pFindData->cFileName));
    pFindData->cFileName[min(NameLen, COUNTOF(pFindData->cFileName))] = 0;
    wcscpy_s(pFindData->cAlternateFileName, sizeof(pFindData->cAlternateFileName), pFindData->cFileName);
}
