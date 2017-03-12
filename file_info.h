#ifndef FILE_INFO_H_INCLUDED
#define FILE_INFO_H_INCLUDED

#include <windows.h>
#include "dokan.h"
#include "internal.h"

int DOKAN_CALLBACK MxfsGetFileInformation(
		LPCWSTR FileName,
		LPBY_HANDLE_FILE_INFORMATION Buffer,
		PDOKAN_FILE_INFO FileInfo);

int DOKAN_CALLBACK MxfsSetFileTime(
		LPCWSTR FileName,
		CONST FILETIME* CreationTime,
		CONST FILETIME* LastAccessTime,
		CONST FILETIME* LastWriteTime,
		PDOKAN_FILE_INFO FileInfo);

void MxfsFillFindData(WIN32_FIND_DATAW *pFindData, const minix_dir_entry *pDirEntry, const minix_inode *pInfo);
void MxfsFileTimeFromTimeT(LPFILETIME FileTime, time_t t);
time_t MxfsTimeTFromFileTime(CONST FILETIME *FileTime);

#endif // FILE_INFO_H_INCLUDED
