#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <windows.h>
#include "partitions.h"
#include "general.h"
#include "debug.h"
#include "dokan.h"
#include "stubs.h"
#include "file.h"
#include "file_info.h"
#include "dir.h"
#include "read_write.h"
#include "inode.h"
#include "volume_info.h"
#include "disk_image.h"

#define VERSION "0.4"

typedef struct
{
    unsigned Part;
    unsigned Subpart;
    const char *Path;
    SUBPARTITION *pSubpart;
    DISK_IMG *pImg;
    HANDLE hEvent;
    volatile BOOL bMounted;
} MOUNT_REQ;

static DOKAN_OPERATIONS DokanCallbacks = {
    MxfsCreateFile, // CreateFile
    MxfsOpenDirectory, // OpenDirectory
    MxfsCreateDirectory, // CreateDirectory
    MxfsCleanup, // Cleanup
    MxfsCloseFile, // CloseFile
    MxfsReadFile, // ReadFile
    MxfsWriteFile, // WriteFile
    MxfsFlushFileBuffers, // FlushFileBuffers
    MxfsGetFileInformation, // GetFileInformation,
    MxfsFindFiles, // FindFiles
    NULL, // FindFilesWithPattern
    MxfsSetFileAttributes, // SetFileAttributes
    MxfsSetFileTime, // SetFileTime
    MxfsDeleteFile, // DeleteFile
    MxfsDeleteDirectory, // DeleteDirectory
    MxfsMoveFile, // MoveFile
    MxfsSetEndOfFile, // SetEndOfFile
    MxfsSetAllocationSize, // SetAllocationSize
    MxfsLockFile, // LockFile
    MxfsUnlockFile, // UnlockFile
    MxfsGetDiskFreeSpace, // GetDiskFreeSpace
    MxfsGetVolumeInformation, // GetVolumeInformation
    MxfsUnmount, // Unmount
    MxfsGetFileSecurity, // GetFileSecurity
    MxfsSetFileSecurity, // SetFileSecurity
};

static HANDLE g_Threads[16];
static unsigned g_cThreads = 0;
static MOUNT_REQ g_MountReq[16];
static unsigned g_cMountReq = 0;

static DWORD WINAPI MxfsThread(LPVOID lpParameter)
{
    MOUNT_REQ *pReq = (MOUNT_REQ*)lpParameter;
    MINIX_FS *pFileSys;
    WCHAR wszMountPath[MAX_PATH];
    
    mbstowcs(wszMountPath, pReq->Path, sizeof(wszMountPath)/sizeof(wszMountPath[0]));
    
    pFileSys = MxfsOpen(pReq->pImg, pReq->pSubpart->Offset, pReq->pSubpart->Size);
    if(!pFileSys)
    {
        ERR("Failed to open Minix filesystem\n");
        SetEvent(pReq->hEvent);
        return (DWORD)-1;
    }
    
    DOKAN_OPTIONS Opts = {
        DOKAN_VERSION, // Version
        1, // ThreadCount, TODO: support many threads...
        DOKAN_OPTION_DEBUG, // Options
        (ULONG64)(LONG_PTR)pFileSys, // GlobalContext
        wszMountPath, // MountPoint
    };
    
    SetEvent(pReq->hEvent);
    pReq->bMounted = TRUE;
    int err = DokanMain(&Opts, &DokanCallbacks);
    if(err < 0)
        ERR("DokanMain failed: %d\n", err);
    pReq->bMounted = FALSE;
    MxfsClose(pFileSys);
    return 0;
}

static int MxfsMount(MOUNT_REQ *pReq)
{
    pReq->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE hThread = CreateThread(NULL, 0, MxfsThread, pReq, 0, NULL);
    if(!hThread)
        return -1;
    
    WaitForSingleObject(pReq->hEvent, INFINITE);
    CloseHandle(pReq->hEvent);
    Sleep(10);
    
    g_Threads[g_cThreads++] = hThread;
    return 0;
}

static LONG WINAPI ExceptionHandler(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    ERR("Exception!!!\n");
    return EXCEPTION_EXECUTE_HANDLER;
}

static BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
    printf("Console Ctrl Handler: unmounting...\n");
    unsigned i;
    WCHAR wszMountPath[MAX_PATH];
    
    for(i = 0; i < g_cMountReq; ++i)
    {
        if(!g_MountReq[i].bMounted)
            continue;
        
        mbstowcs(wszMountPath, g_MountReq[i].Path, sizeof(wszMountPath)/sizeof(wszMountPath[0]));
        BOOL Result = DokanRemoveMountPoint(wszMountPath);
        if(!Result)
            ERR("DokanRemoveMountPoint(%ls) failed!\n", wszMountPath);
    }
    
    return TRUE;
}

static void DisplayHelp(const char *pProcName)
{
    printf("Usage: %s [-l] [-f img_path p0s0=mount_path1 p0s1=mount_path2 ...]\n", pProcName);
    printf("-l\t\t\tLists all partitions and subpartitions\n");
    printf("-f img_path\t\tSets disk image path\n");
    printf("p0s0=mount_path\t\tMounts subpartition 0 on partition 0 in given path\n");
}

static void Test()
{
#ifndef NDEBUG
    FILETIME ft;
    time_t t = time(NULL);
    MxfsFileTimeFromTimeT(&ft, t);
    assert(t == MxfsTimeTFromFileTime(&ft));
#endif
}

int main(int argc, char *argv[])
{
    const char *pszPath = NULL;
    unsigned i;
    BOOL bHelp = FALSE, bList = FALSE;
    
    printf("MINIX Filesystem Driver " VERSION " for Windows (c) 2013 Rafal Harabien\n");
    
    Test();
    
    for(i = 1; i < (unsigned)argc; ++i)
    {
        if(!strcmp(argv[i], "-f") && i + 1 < (unsigned)argc)
            pszPath = argv[++i];
        else if(!strcmp(argv[i], "-l"))
            bList = TRUE;
        else if(!strcmp(argv[i], "-h"))
        {
            bHelp = TRUE;
            break;
        }
        else
        {
            unsigned uPart, uSubpart;
            if(sscanf(argv[i], "p%us%u=", &uPart, &uSubpart) == 2 && g_cMountReq < COUNTOF(g_MountReq))
            {
                const char *pszMount = strchr(argv[i], '=') + 1;
                g_MountReq[g_cMountReq].Part = uPart;
                g_MountReq[g_cMountReq].Subpart = uSubpart;
                g_MountReq[g_cMountReq].Path = pszMount;
                g_MountReq[g_cMountReq].bMounted = FALSE;
                ++g_cMountReq;
            } else
                printf("Invalid argument: %s\n", argv[i]);
        }
    }
    
    if(!pszPath)
        bHelp = TRUE;
    if(bHelp)
    {
        DisplayHelp(argv[0]);
        return 0;
    }
    
    DISK_IMG *pImg = ImgOpen(pszPath);
    if(!pImg)
    {
        ERR("Failed to open image: %s\n", pszPath);
        return -1;
    }
    
    SUBPART_VECTOR SubpartVect;
    int err = FindSubpartitions(pImg, &SubpartVect);
    if(err < 0)
    {
        ERR("FindSubpartitions failed!\n");
        ImgClose(pImg);
        return err;
    }
    
    if(bList)
    {
        printf("Subpartitions:\n");
        for(i = 0; i < SubpartVect.Count; ++i)
        {
            SUBPARTITION *pSubpart = &SubpartVect.Entries[i];
            printf("p%us%u - offset 0x%x size 0x%x\n", pSubpart->PartIdx, pSubpart->SubpartIdx, pSubpart->Offset, pSubpart->Size);
        }
    }
    
    SetUnhandledExceptionFilter(ExceptionHandler);
    
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    
    for(i = 0; i < g_cMountReq; ++i)
    {
        unsigned j;
        for(j = 0; j < SubpartVect.Count; ++j)
        {
            if(g_MountReq[i].Part == SubpartVect.Entries[j].PartIdx && g_MountReq[i].Subpart == SubpartVect.Entries[j].SubpartIdx)
                break;
        }
        
        if(j < SubpartVect.Count)
        {
            printf("Mounting p%us%u in %s...\n", g_MountReq[i].Part, g_MountReq[i].Subpart, g_MountReq[i].Path);
            g_MountReq[i].pSubpart = &SubpartVect.Entries[j];
            g_MountReq[i].pImg = pImg;
            MxfsMount(&g_MountReq[i]);
        }
    }
    
    WaitForMultipleObjects(g_cThreads, g_Threads, TRUE, INFINITE);
    for(i = 0; i < g_cThreads; ++i)
        CloseHandle(g_Threads[i]);
    
    MxfsFree(SubpartVect.Entries);
    ImgClose(pImg);
    
    printf("Done!\n");
    return 0;
}
