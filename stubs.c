#include "stubs.h"
#include "general.h"
#include "debug.h"
#include "internal.h"

int DOKAN_CALLBACK MxfsMoveFile(
		LPCWSTR ExistingFileName,
		LPCWSTR NewFileName,
		BOOL ReplaceExisiting,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls %ls %u", ExistingFileName, NewFileName, ReplaceExisiting ? 1 : 0);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsLockFile(
		LPCWSTR FileName,
		LONGLONG ByteOffset,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls %I64u %I64u", FileName, ByteOffset, Length);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsUnlockFile(
		LPCWSTR FileName,
		LONGLONG ByteOffset,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls %I64u %I64u", FileName, ByteOffset, Length);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsUnmount(
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("");
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

// Optional
int DOKAN_CALLBACK MxfsFlushFileBuffers(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls", FileName);
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    MxfsCacheFlush(FileSys); // flush entire FS...
    return 0;
}

int DOKAN_CALLBACK MxfsSetFileAttributes(
		LPCWSTR FileName,
		DWORD FileAttributes,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("%ls", FileName);
    return 0;
}

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
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsDeleteDirectory(
		LPCWSTR FileName,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsSetAllocationSize(
		LPCWSTR FileName,
		LONGLONG Length,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

// Suported since 0.6.0. You must specify the version at DOKAN_OPTIONS.Version.
int DOKAN_CALLBACK MxfsGetFileSecurity(
		LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo, // A pointer to SECURITY_INFORMATION value being requested
		PSECURITY_DESCRIPTOR SecurityDescriptor, // A pointer to SECURITY_DESCRIPTOR buffer to be filled
		ULONG SecurityDescriptorLength, // length of Security descriptor buffer
		PULONG LengthNeeded,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}

int DOKAN_CALLBACK MxfsSetFileSecurity(
		LPCWSTR FileName,
		PSECURITY_INFORMATION SecurityInfo,
		PSECURITY_DESCRIPTOR SecurityDescriptor,
		ULONG SecurityDescriptorLength,
		PDOKAN_FILE_INFO FileInfo)
{
    UNIMPLEMENTED("%ls", FileName);
    return -ERROR_CALL_NOT_IMPLEMENTED;
}
