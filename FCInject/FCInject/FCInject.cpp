// FCInject.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 测试注册表程序

#include <iostream>
#include <windows.h>

#include "detours.h"
#pragma comment(lib, "detours.lib")

int main(){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);

	// 设置Dll文件的路径
	WCHAR DirPath[MAX_PATH + 1];
	wcscpy_s(DirPath, MAX_PATH, L"C:\\Users\\Leo\\Documents\\Gitee\\hustcse-apihook\\hookDll\\x64\\Debug");
	char DLLPath[MAX_PATH + 1] = "C:\\Users\\Leo\\Documents\\Gitee\\hustcse-apihook\\hookDll\\x64\\Debug\\hookDll.dll";
	
	// 设置需要注入程序的地址
	WCHAR EXE[MAX_PATH + 1] = { 0 };
	// wcscpy_s(EXE, MAX_PATH, L"C:\\Users\\Leo\\Documents\\Gitee\\hustcse-apihook\\test_exe\\filetest.exe");
	// wcscpy_s(EXE, MAX_PATH, L"C:\\Users\\Leo\\Documents\\Gitee\\hustcse-apihook\\test_exe\\heaptest.exe");
	wcscpy_s(EXE, MAX_PATH, L"C:\\Users\\Leo\\Documents\\Gitee\\hustcse-apihook\\test_exe\\regtest.exe");

	if (DetourCreateProcessWithDllEx(EXE, NULL, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, DirPath, &si, &pi, DLLPath, NULL)) {
		// MessageBoxA(NULL, "INJECT", "INJECT", NULL);
		ResumeThread(pi.hThread);
		WaitForSingleObject(pi.hThread, INFINITE);
	}
	else {
		printf("ERROR!\n");
		char error[100];
		sprintf_s(error, "%d", GetLastError());
	}
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
