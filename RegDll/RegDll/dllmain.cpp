/*
* dllmain.cpp : 定义 DLL 应用程序的入口点。
* 实现注册表相关内容监控
* 注册表相关的API函数功能参考：https://learn.microsoft.com/zh-cn/windows/win32/api/winreg/
*/ 

#include "pch.h"
#include "framework.h"
#include "detours.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "detours.lib")

SYSTEMTIME st;

// 定义和引入需要Hook的函数和替换的函数
static LSTATUS(WINAPI* OldRegCreateKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) = RegCreateKeyExA;
static LSTATUS(WINAPI* OldRegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) = RegOpenKeyExA;
static LSTATUS(WINAPI* OldRegCloseKey)(HKEY hKey) = RegCloseKey;
static LSTATUS(WINAPI* OldRegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) = RegQueryValueExA;
static LSTATUS(WINAPI* OldRegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) = RegSetValueExA;

// 函数声明
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCloseKey(HKEY hKey);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE * lpData, DWORD cbData);

// 创建注册表的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCreateKeyExA(
	HKEY hKey,												// 注册表主键的句柄
	LPCSTR lpSubKey,										// 子键的名称
	DWORD Reserved,											// 保留，必须为0
	LPSTR lpClass,											// 对象类型，通常为NULL
	DWORD dwOptions,										// 用于设置新键的选项
	REGSAM samDesired,										// 访问权限标志
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,		// 安全属性
	PHKEY phkResult,										// 接收新键的句柄
	LPDWORD lpdwDisposition									// 接收操作结果的指针
) {
	GetLocalTime(&st);
	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	/*
	* RegOpenKeyExA函数：
	* 打开一个已存在的注册表项，返回值为ERROR_SUCCESS表示打开成功。
	*/ 

	// 调用RegOpenKeyExA函数打开指定的注册表项
	auto judge = OldRegOpenKeyExA(hKey, lpSubKey, 0, samDesired, phkResult);

	// 返回值不为ERROR_SUCCESS，说明指定的注册表项在函数调用的时候并不存在，因此认为程序存在新增注册表项操作
	if (judge != ERROR_SUCCESS) {
		printf("----------------------------------------\n");
		printf("[WARNING] 此程序存在新增注册表项操作！\n");
		printf("----------------------------------------\n");
	}

	/*
	* RegCreateKeyExA函数：
	* 创建指定的注册表项。如果该项已存在，函数将打开它。
	*/

	// 调用RegCreateKeyExA函数创建或打开指定的注册表项
	auto status = OldRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

	/*
	* SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run是一个特殊的注册表项。
	* 这个注册表项包含了一系列的键值对，每一个键值对都对应一个需要在系统启动时自动运行的程序。
	*/

	// 对比打开的子键名称。若子键为"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"，则说明程序很有可能要修改自启动项。
	if (strcmp(lpSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") == 0) {
		printf("----------------------------------------\n");
		printf("[WARNING] 此程序存在修改注册表自启动项的操作！\n");
		printf("----------------------------------------\n");
	}

	// 打印其他参数
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpSubKey: " << lpSubKey << std::endl;
	std::cout << "Reserved: " << Reserved << std::endl;
	if (lpClass == NULL) {
		std::cout << "lpClass: " << "NULL" << std::endl;
	}
	std::cout << "dwOptions: " << dwOptions << std::endl;
	std::cout << "samDesired: " << samDesired << std::endl;
	if (lpSecurityAttributes == NULL) {
		std::cout << "lpSecurityAttributes: " << "NULL" << std::endl;
	}
	std::cout << "phkResult: " << phkResult << std::endl;
	if (lpdwDisposition == NULL) {
		std::cout << "lpdwDisposition: " << "NULL" << std::endl;
	}

	printf("****************************************\n\n");

	return status;
}

// 打开注册表的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegOpenKeyExA(
	HKEY hKey,				// 注册表主键的句柄
	LPCSTR lpSubKey,		// 子键的名称
	DWORD ulOptions,		// 用于设置打开键的选项
	REGSAM samDesired,		// 访问权限标志
	PHKEY phkResult			// 接收打开的键的句柄
) {
	GetLocalTime(&st);
	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	auto status = OldRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpSubKey: " << lpSubKey << std::endl;
	std::cout << "ulOptions: " << ulOptions << std::endl;
	std::cout << "samDesired: " << samDesired << std::endl;
	std::cout << "phkResult: " << phkResult << std::endl;

	printf("****************************************\n\n");

	return status;
}

// 关闭注册表的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCloseKey(
	HKEY hKey				// 注册表主键的句柄
) {
	GetLocalTime(&st);
	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	auto status = OldRegCloseKey(hKey);
	std::cout << "hKey: " << hKey << std::endl;

	printf("****************************************\n\n");
	
	return status;
}

// 查询注册表键值的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegQueryValueExA(
	HKEY hKey,				// 注册表主键的句柄
	LPCSTR lpValueName,		// 值的名称
	LPDWORD lpReserved,		// 保留，必须为NULL
	LPDWORD lpType,			// 接受值类型的指针
	LPBYTE lpData,			// 接收值数据的缓冲区的指针
	LPDWORD lpcbData		// 缓冲区的大小，以字节为单位
) {
	GetLocalTime(&st);
	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	auto status = OldRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "lpReserved: " << lpReserved << std::endl;
	std::cout << "lpType: " << lpType << std::endl;
	std::cout << "lpData: " << lpData << std::endl;
	std::cout << "lpcbData: " << lpcbData << std::endl;

	printf("****************************************\n\n");

	return status;
}

// 修改注册表键值的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegSetValueExA(
	HKEY hKey,				// 注册表主键的句柄
	LPCSTR lpValueName,		// 值的名称
	DWORD Reserved,			// 保留，必须为0
	DWORD dwType,			// 值的类型
	const BYTE* lpData,		// 指向包含值数据的缓冲区的指针
	DWORD cbData			// 缓冲区的大小，以字节为单位
) {
	GetLocalTime(&st);
	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	/*
	* RegSetValueExA函数：
	* 设置注册表项下指定值的数据和类型。
	*/

	// 调用RegSetValueExA函数设置指定的注册表项的值，并将返回的状态保存在status变量中
	auto status = OldRegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);

	// 如果返回值为ERROR_SUCCESS，说明函数成功执行（也就是修改了注册表项的值）
	if (status == ERROR_SUCCESS) {
		printf("----------------------------------------\n");
		printf("[WARNING] 此程序存在修改注册表项的操作！\n");
		printf("----------------------------------------\n");
	}

	// 打印其他参数
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "Reserved: " << Reserved << std::endl;
	std::cout << "dwType: " << dwType << std::endl;
	std::cout << "lpData: " << lpData << std::endl;
	std::cout << "cbData: " << cbData << std::endl;

	printf("****************************************\n\n");

	return status;
}

// 主函数
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)OldRegCreateKeyExA, NewRegCreateKeyExA);
		DetourAttach(&(PVOID&)OldRegOpenKeyExA, NewRegOpenKeyExA);
		DetourAttach(&(PVOID&)OldRegQueryValueExA, NewRegQueryValueExA);
		DetourAttach(&(PVOID&)OldRegSetValueExA, NewRegSetValueExA);
		DetourAttach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);

		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID&)OldRegCreateKeyExA, NewRegCreateKeyExA);
		DetourDetach(&(PVOID&)OldRegOpenKeyExA, NewRegOpenKeyExA);
		DetourDetach(&(PVOID&)OldRegQueryValueExA, NewRegQueryValueExA);
		DetourDetach(&(PVOID&)OldRegSetValueExA, NewRegSetValueExA);
		DetourDetach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);

		DetourTransactionCommit();
		break;
	}
    return TRUE;
}
