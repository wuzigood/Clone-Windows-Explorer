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

/*
文件相关的基本操作：
复制，移动，重命名，新建文件夹，新建文件
*/

extern "C" _declspec(dllexport) void CopyFileTo()
{

}

extern "C" _declspec(dllexport) void MoveFileTo()
{

}

extern "C" _declspec(dllexport) void RenameFile()
{

}

extern "C" _declspec(dllexport) void NewFolder()
{

}

extern "C" _declspec(dllexport) void NewFile()
{

}

/*
文件的搜索，两个函数，一个用于初始化搜索，另一个用于搜索下一个
InitSearch：如果第一次搜索不到，那么返回FALSE；之后也不会搜索到
NextSearch：搜索下一个，返回下个搜索项。如果已经找不到了，那么返回FALSE;

多线程的搜索方案：
搞一个数组，存放搜索的结果。每次搜到了内容就往里面给放，计数。
NextSearch不做实际工作，就是返回内容给调用者。当到达计数值的时候，就返回FALSE；
*/

// search_count 应该设置成原子变量
// 一次只能有一个线程操作这个变量。这里需要进程的互斥。
int search_count = 0;
int return_count = 0;
const int MaxCount = 1000;
FileInfo searchResult[MaxCount];

static void internal_search()
{

}

extern "C" _declspec(dllexport) BOOL InitSearch()
{
	return FALSE;
}

extern "C" _declspec(dllexport) BOOL NextSearch(FileInfo &fileInfo)
{
	if (return_count < search_count) {
		fileInfo = searchResult[return_count];
		return_count++;
		return TRUE;
	}
	return FALSE;
}

extern "C" _declspec(dllexport) DWORD GetLogicalDrivesNumber()
{
	return GetLogicalDrives();
}