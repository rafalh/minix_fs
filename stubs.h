#ifndef STUBS_H_INCLUDED
#define STUBS_H_INCLUDED

#include <windows.h>
#include "dokan.h"

int DOKAN_CALLBACK MxfsMoveFile(
		LPCWSTR ExistingFileName,
		LPCWSTR NewFileName,
		BOOL ReplaceExisiting,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsLockFile(
		LPCWSTR FileName,
		LONGLONG ByteOffset,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsUnlockFile(
		LPCWSTR FileName,
		LONGLONG ByteOffset,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsUnmount(
		PDOKAN_FILE_INFO FileInfo);

// Optional
int DOKAN_CALLBACK MxfsFlushFileBuffers(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsSetFileAttributes(
		LPCWSTR FileName,
		DWORD FileAttributes,
		PDOKAN_FILE_INFO FileInfo);

// You should not delete file on DeleteFile or DeleteDirectory.
// When DeleteFile or DeleteDirectory, you must check whether
// you can delete the file or not, and return 0 (when you can delete it)
// or appropriate error codes such as -ERROR_DIR_NOT_EMPTY,
// -ERROR_SHARING_VIOLATION.
// When you return 0 (ERROR_SUCCESS), you get Cleanup with
// FileInfo->DeleteOnClose set TRUE and you have to delete the
// file in Close.
int DOKAN_CALLBACK MxfsDeleteFile(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsDeleteDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsSetAllocationSize(
		LPCWSTR FileName,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo);

// Suported since 0.6.0. You must specify the version at DOKAN_OPTIONS.Version.
int DOKAN_CALLBACK MxfsGetFileSecurity(
		LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo, // A pointer to SECURITY_INFORMATION value being requested
		PSECURITY_DESCRIPTOR SecurityDescriptor, // A pointer to SECURITY_DESCRIPTOR buffer to be filled
		ULONG SecurityDescriptorLength, // length of Security descriptor buffer
		PULONG LengthNeeded,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsSetFileSecurity(
		LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo,
		PSECURITY_DESCRIPTOR SecurityDescriptor,
		ULONG SecurityDescriptorLength,
		PDOKAN_FILE_INFO FileInfo);

#endif // STUBS_H_INCLUDED
