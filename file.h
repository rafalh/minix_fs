#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <windows.h>
#include "dokan.h"
#include "internal.h"

int DOKAN_CALLBACK MxfsCreateFile(
		LPCWSTR FileName,
		DWORD DesiredAccess,
		DWORD ShareMode,
		DWORD CreationDisposition,
		DWORD FlagsAndAttributes,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsCreateDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsOpenDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsCleanup(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsCloseFile(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

typedef struct
{
    int Index;
    unsigned Length;
    BOOL Write, Changed;
    //minix_inode Info;
#ifndef NDEBUG
    WCHAR DbgBuf[256];
#endif
} FILE_CTX;

#endif // FILE_H_INCLUDED
