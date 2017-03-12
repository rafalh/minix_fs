#include <stdlib.h>
#include "disk_image.h"
#include "general.h"

DISK_IMG *ImgOpen(const char *pszPath)
{
    HANDLE FileHandle = CreateFileA(pszPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(FileHandle == INVALID_HANDLE_VALUE)
        return NULL;
    
    DISK_IMG *pImg = MxfsAlloc(sizeof(DISK_IMG));
    if(!pImg)
        return NULL;
    
    pImg->FileHandle = FileHandle;
    InitializeCriticalSection(&pImg->Lock);
    return pImg;
}

void ImgClose(DISK_IMG *pImg)
{
    if(!pImg) return;
    CloseHandle(pImg->FileHandle);
    MxfsFree(pImg);
}

int ImgRead(DISK_IMG *pImg, unsigned uOffset, unsigned cBytes, PVOID pBuf)
{
    DWORD dwBytesRead;
    BOOL bSuccess;
    
    EnterCriticalSection(&pImg->Lock);
    SetFilePointer(pImg->FileHandle, uOffset, NULL, FILE_BEGIN);
    bSuccess = ReadFile(pImg->FileHandle, pBuf, cBytes, &dwBytesRead, NULL);
    LeaveCriticalSection(&pImg->Lock);
    
    if(!bSuccess || dwBytesRead != cBytes)
        return -ERROR_READ_FAULT;
    
    return 0;
}

int ImgWrite(DISK_IMG *pImg, unsigned uOffset, unsigned cBytes, PVOID pBuf)
{
    DWORD dwBytesWritten = 0;
    BOOL bSuccess = FALSE;
    
    EnterCriticalSection(&pImg->Lock);
    if(uOffset + cBytes <= GetFileSize(pImg->FileHandle, NULL))
    {
        SetFilePointer(pImg->FileHandle, uOffset, NULL, FILE_BEGIN);
        bSuccess = WriteFile(pImg->FileHandle, pBuf, cBytes, &dwBytesWritten, NULL);
    }
    LeaveCriticalSection(&pImg->Lock);
    
    if(!bSuccess || dwBytesWritten != cBytes)
        return -ERROR_WRITE_FAULT;
    
    return 0;
}

unsigned ImgGetSize(DISK_IMG *pImg)
{
    EnterCriticalSection(&pImg->Lock);
    unsigned uSize = GetFileSize(pImg->FileHandle, NULL);
    LeaveCriticalSection(&pImg->Lock);
    return uSize;
}
