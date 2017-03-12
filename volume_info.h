#ifndef VOLUME_INFO_H_INCLUDED
#define VOLUME_INFO_H_INCLUDED

#include <windows.h>
#include "dokan.h"

// Neither GetDiskFreeSpace nor GetVolumeInformation
// save the DokanFileContext->Context.
// Before these methods are called, CreateFile may not be called.
// (ditto CloseFile and Cleanup)

// see Win32 API GetDiskFreeSpaceEx
int DOKAN_CALLBACK MxfsGetDiskFreeSpace(
		PULONGLONG FreeBytesAvailable,
		PULONGLONG TotalNumberOfBytes,
		PULONGLONG TotalNumberOfFreeBytes,
		PDOKAN_FILE_INFO FileInfo);


// see Win32 API GetVolumeInformation
int DOKAN_CALLBACK MxfsGetVolumeInformation(
		LPWSTR VolumeNameBuffer,
		DWORD VolumeNameSize, // in num of chars
		LPDWORD VolumeSerialNumber,
		LPDWORD MaximumComponentLength, // in num of chars
		LPDWORD FileSystemFlags,
		LPWSTR FileSystemNameBuffer,
		DWORD FileSystemNameSize, // in num of chars
		PDOKAN_FILE_INFO FileInfo);

#endif // VOLUME_INFO_H_INCLUDED
