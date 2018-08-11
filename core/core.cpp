// core.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <iostream>

extern "C" _declspec(dllexport) int Sum(int a, int b)
{
	return a + b;
}

const unsigned int length = 100;
TCHAR path[length];

extern "C" _declspec(dllexport) TCHAR* GetCurrentDir()
{
	int ret = GetCurrentDirectory(length, path);
	return path;
}