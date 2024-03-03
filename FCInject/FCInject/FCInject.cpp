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
	// wcscpy_s(EXE, MAX_PATH, L"E:\\security_task\\heaptest\\x64\\Debug\\heaptest.exe");
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