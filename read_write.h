#ifndef READ_WRITE_H_INCLUDED
#define READ_WRITE_H_INCLUDED

#include "general.h"
#include "dokan.h"

int MxfsReadBlocks(MINIX_FS *FileSys, void *pBuffer, unsigned Block, unsigned Count);

int DOKAN_CALLBACK MxfsReadFile(
		LPCWSTR FileName,
		LPVOID Buffer,
		DWORD NumberOfBytesToRead,
		LPDWORD NumberOfBytesRead,
		LONGLONG Offset,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsWriteFile(
		LPCWSTR FileName,
		LPCVOID Buffer,
		DWORD NumberOfBytesToWrite,
		LPDWORD NumberOfBytesWritten,
		LONGLONG Offset,
		PDOKAN_FILE_INFO FileInfo);

#endif // READ_WRITE_H_INCLUDED
