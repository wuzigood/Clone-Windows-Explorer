// core.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <iostream>

extern "C" _declspec(dllexport) int Sum(int a, int &b)
{
	b = 10;
	return a + b;
}
