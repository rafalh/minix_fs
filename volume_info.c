#include "volume_info.h"
#include "debug.h"
#include "general.h"

unsigned MxfsGetFreeSpace(MINIX_FS *FileSys)
{
    unsigned UsedZones = MxfsCountBitsSet(&FileSys->ZoneMap);
    unsigned TotalZones = FileSys->Super.s_zones;
    unsigned FreeBlocks = (TotalZones - UsedZones) << FileSys->Super.s_log_zone_size;
    return FreeBlocks * MINIX_BLOCK_SIZE;
}

int DOKAN_CALLBACK MxfsGetDiskFreeSpace(
		PULONGLONG FreeBytesAvailable,
		PULONGLONG TotalNumberOfBytes,
		PULONGLONG TotalNumberOfFreeBytes,
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("");
    
    MINIX_FS *FileSys = (MINIX_FS*)(ULONG_PTR)FileInfo->DokanOptions->GlobalContext;
    ASSERT(FileSys);
    
    unsigned FreeBytes = 0;
    if(FreeBytesAvailable || TotalNumberOfFreeBytes)
        FreeBytes = MxfsGetFreeSpace(FileSys);
    
    if(FreeBytesAvailable)
        *FreeBytesAvailable = FreeBytes;
    if(TotalNumberOfBytes)
        *TotalNumberOfBytes = FileSys->cbLen;
    if(TotalNumberOfFreeBytes)
        *TotalNumberOfFreeBytes = FreeBytes;
    return 0;
}

// see Win32 API GetVolumeInformation
int DOKAN_CALLBACK MxfsGetVolumeInformation(
		LPWSTR VolumeNameBuffer,
		DWORD VolumeNameSize, // in num of chars
		LPDWORD VolumeSerialNumber,
		LPDWORD MaximumComponentLength, // in num of chars
		LPDWORD FileSystemFlags,
		LPWSTR FileSystemNameBuffer,
		DWORD FileSystemNameSize, // in num of chars (this is not true?)
		PDOKAN_FILE_INFO FileInfo)
{
    TRACE("");
    
    if(VolumeNameSize > 0 && VolumeNameBuffer)
        wcscpy_s(VolumeNameBuffer, VolumeNameSize, L"");
    
    if(VolumeSerialNumber)
        *VolumeSerialNumber = MINIX_VOLUME_ID;
    
    if(MaximumComponentLength)
        *MaximumComponentLength = MAX_NAME_LEN;
    
    if(FileSystemFlags)
        *FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH;
    
    if(FileSystemNameSize > 0 && FileSystemNameBuffer)
        wcscpy_s(FileSystemNameBuffer, FileSystemNameSize, L"Minix");
    
    return 0;
}
