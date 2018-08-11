// core.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <iostream>

const unsigned int length = 260;
TCHAR path[length];
WIN32_FIND_DATA fdd;
HANDLE handle = INVALID_HANDLE_VALUE;

extern "C" _declspec(dllexport) int Sum(int a, int b)
{
	return a + b;
}

extern "C" _declspec(dllexport) TCHAR* GetCurrentDir()
{
	int ret = GetCurrentDirectory(length, path);
	return path;
}

struct FileInfo {
	BOOL IsFolder;
	//char Name[length];
	WCHAR Name[length];
};

static void setSearchPath()
{
	TCHAR slash = '\\';
	TCHAR star = '*';
	int i = 0;
	while (i < length && path[i] != '\0') {
		i++;
	}
	path[i++] = slash;
	path[i++] = star;
	path[i] = '\0';
}

extern "C" _declspec(dllexport) BOOL FirstFind(FileInfo &fileInfo)
{
	setSearchPath();
	handle = FindFirstFile(path, &fdd);

	if (handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	fileInfo.IsFolder = (fdd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	memcpy(fileInfo.Name, fdd.cFileName, length);
	return TRUE;
}

extern "C" _declspec(dllexport) BOOL NextFind(FileInfo &fileInfo)
{
	BOOL hasNext = FindNextFile(handle, &fdd);
	if (hasNext) {
		fileInfo.IsFolder = (fdd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		memcpy(fileInfo.Name, fdd.cFileName, 100);
	}
	return hasNext;
}

extern "C" _declspec(dllexport) void SetPath(TCHAR *val)
{
	memcpy(path, val, length);
}