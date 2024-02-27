// dllmain.cpp : 定义 DLL 应用程序的入口点。
// 注册表相关内容监控

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

extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCreateKeyExA(
	HKEY hKey,
	LPCSTR lpSubKey,
	DWORD Reserved,
	LPSTR lpClass,
	DWORD dwOptions,
	REGSAM samDesired,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY phkResult,
	LPDWORD lpdwDisposition)
{
	GetLocalTime(&st);

	printf("\n\n****************************************\n");
	printf("RegCreateKeyExA Hooked\n");
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	printf("****************************************\n\n");
	
	auto judge = OldRegOpenKeyExA(hKey, lpSubKey, 0, samDesired, phkResult);
	if (judge != ERROR_SUCCESS) {
		printf("----------------------------------------\n");
		printf("Hint: 此程序存在新增注册表项操作\n");
		printf("----------------------------------------\n");
	}

	auto status = OldRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpSubKey: " << lpSubKey << std::endl;

	if (strcmp(lpSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") == 0)
		//发现在修改自启动项
	{
		std::cout << "\n-------------------------------------------\n";
		std::cout << "warning! 此程序存在修改注册表自启动项的操作";
		std::cout << "\n-------------------------------------------\n";
	}

	std::cout << "Reserved: " << Reserved << std::endl;
	if (lpClass == NULL) std::cout << "lpClass: " << "NULL" << std::endl;
	std::cout << "dwOptions: " << dwOptions << std::endl;
	std::cout << "samDesired: " << samDesired << std::endl;
	if (lpSecurityAttributes == NULL) std::cout << "lpSecurityAttributes: " << "NULL" << std::endl;
	std::cout << "phkResult: " << phkResult << std::endl;
	if (lpdwDisposition == NULL) std::cout << "lpdwDisposition: " << "NULL" << std::endl;
	std::cout << "********************************************\n\n";

	return status;
}

extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegOpenKeyExA(
	HKEY hKey,
	LPCSTR lpSubKey,
	DWORD ulOptions,
	REGSAM samDesired,
	PHKEY phkResult)
{
	auto status = OldRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);

	std::cout << "\n********************************************\n";
	printf("RegOpenKeyExA Hooked\n");
	GetLocalTime(&st);
	std::cout << "DLL日志输出: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":" << st.wMilliseconds << std::endl;
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpSubKey: " << lpSubKey << std::endl;
	std::cout << "ulOptions: " << ulOptions << std::endl;
	std::cout << "samDesired: " << samDesired << std::endl;
	std::cout << "phkResult: " << phkResult << std::endl;
	std::cout << "\n********************************************\n";
	
	return status;
}


extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCloseKey(HKEY hKey) {
	auto status = OldRegCloseKey(hKey);
	
	std::cout << "\n********************************************\n";
	std::cout << "RegCloseKey Hooked\n";
	GetLocalTime(&st);
	std::cout << "DLL日志输出: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":" << st.wMilliseconds << std::endl;
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "\n********************************************\n";
	
	return status;
}


extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegQueryValueExA(
	HKEY hKey,
	LPCSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData)
{
	auto status = OldRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	
	std::cout << "\n********************************************\n";
	std::cout << "RegQueryValueExA Hooked\n";
	GetLocalTime(&st);
	std::cout << "DLL日志输出: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":" << st.wMilliseconds << std::endl;
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "lpReserved: " << lpReserved << std::endl;
	std::cout << "lpType: " << lpType << std::endl;
	std::cout << "lpData: " << lpData << std::endl;
	std::cout << "lpcbData: " << lpcbData << std::endl;
	std::cout << "\n********************************************\n";
	
	return status;
}

extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegSetValueExA(
	HKEY hKey,
	LPCSTR lpValueName,
	DWORD Reserved,
	DWORD dwType,
	const BYTE * lpData,
	DWORD cbData)
{
	auto status = OldRegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
	std::cout << "\n********************************************\n";
	std::cout << "RegSetValueExA Hooked\n";
	GetLocalTime(&st);
	std::cout << "DLL日志输出: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":" << st.wMilliseconds << std::endl;
	if (status == ERROR_SUCCESS) {
		std::cout << "\n-------------------------------------------\n";
		std::cout << "Hint: 此程序存在修改注册表项操作";
		std::cout << "\n-------------------------------------------\n";
	}
	

	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "Reserved: " << Reserved << std::endl;
	std::cout << "dwType: " << dwType << std::endl;
	std::cout << "lpData: " << lpData << std::endl;
	std::cout << "cbData: " << cbData << std::endl;
	std::cout << "\n********************************************\n";
	
	return status;
}


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

