// FCInject.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 测试注册表程序

#include <iostream>
#include <windows.h>

#include "detours.h"
#pragma comment(lib, "detours.lib")

using namespace std;

int main(){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);

	// 设置Dll文件的路径
	WCHAR DirPath[MAX_PATH + 1];
	cout << "请输入Dll文件所在的文件夹路径: " << endl;
	wcin >> DirPath;
	char DLLPath[MAX_PATH + 1];
	cout << "请输入Dll文件的完整路径: " << endl;
	cin >> DLLPath;
	
	// 设置需要注入程序的地址
	WCHAR EXE[MAX_PATH + 1];
	cout << "请输入需要注入程序的地址: " << endl;
	wcin >> EXE;

	if (DetourCreateProcessWithDllEx(EXE, NULL, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, DirPath, &si, &pi, DLLPath, NULL)) {
		ResumeThread(pi.hThread);
		WaitForSingleObject(pi.hThread, INFINITE);
	}
	else {
		printf("ERROR!\n");
		printf("Error Code: %d", GetLastError());
	}

	return 0;
}