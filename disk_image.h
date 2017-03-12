#ifndef DISK_IMAGE_H_INCLUDED
#define DISK_IMAGE_H_INCLUDED

#include <windows.h>
#include <stdio.h>

typedef struct
{
    CRITICAL_SECTION Lock;
    HANDLE FileHandle;
} DISK_IMG;

DISK_IMG *ImgOpen(const char *pszPath);
void ImgClose(DISK_IMG *pImg);
int ImgRead(DISK_IMG *pImg, unsigned uOffset, unsigned cBytes, PVOID pBuf);
int ImgWrite(DISK_IMG *pImg, unsigned uOffset, unsigned cBytes, PVOID pBuf);
unsigned ImgGetSize(DISK_IMG *pImg);

#endif // DISK_IMAGE_H_INCLUDED
