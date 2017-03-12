#ifndef DIR_H_INCLUDED
#define DIR_H_INCLUDED

#include <windows.h>
#include "dokan.h"
#include "general.h"

int DOKAN_CALLBACK MxfsFindFiles(
		LPCWSTR PathName,
		PFillFindData Callback,
		PDOKAN_FILE_INFO FileInfo);

int MxfsParsePath(MINIX_FS *pFileSys, const char *pszPath, BOOL Parent);

#endif // DIR_H_INCLUDED
