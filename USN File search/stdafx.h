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
using namespace std;

struct USN_FileRecord
{
	//string FileName;
	WCHAR *FileName;
	//CString FileName;
	DWORDLONG FileReferenceNumber;
	DWORDLONG ParentFileReferenceNumber;
};

// TODO: 在此处引用程序需要的其他头文件
