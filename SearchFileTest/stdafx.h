// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <winioctl.h>
#include <vector>
#include <string.h>
#include <unordered_map>
#include <map>
using namespace std;

/*
使用map还是unordered_map需要抉择。
如果使用unordered_map，需要预留空间，否则将会比map更耗时间
*/

struct USN_FileRecord
{
	WCHAR *FileName;
	// Just save a bit space, 1000,000 about 18MB 
	 DWORDLONG FileReferenceNumber; 
	DWORDLONG ParentFileReferenceNumber;
};

struct ResultFindFile
{
	WCHAR FileName[MAX_PATH];
	WCHAR Path[MAX_PATH];
};
// TODO: 在此处引用程序需要的其他头文件
