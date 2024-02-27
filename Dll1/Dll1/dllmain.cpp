// dllmain.cpp : 定义 DLL 应用程序的入口点。
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
static int (WINAPI* OldMessageBoxA)(_In_opt_ HWND hwnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType) = MessageBoxA;

extern "C" __declspec(dllexport)int WINAPI NewMessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType) {
	printf("\n\n****************************************\n");
	printf("MessageBoxA Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	printf("****************************************\n\n");
	return OldMessageBoxA(NULL, "new MessageBoxA", "Hooked", MB_OK);
}

static int (WINAPI* OldMessageBoxW)(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType) = MessageBoxW;

extern "C" __declspec(dllexport)int WINAPI NewMessageBoxW(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType) {
	printf("\n\n****************************************\n");
	printf("MessageBoxW Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	printf("****************************************\n\n");
	return OldMessageBoxW(NULL, L"new MessageBoxW", L"Hooked", MB_OK);
}

// 文件操作 OpenFile CreateFile
static HANDLE(WINAPI * OldCreateFile)(
	LPCTSTR lpFileName,							// 文件名
	DWORD dwDesiredAccess,						// 访问模式
	DWORD dwShareMode,							// 共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, // 安全属性(也即销毁方式)
	DWORD dwCreationDisposition,				// how to create
	DWORD dwFlagsAndAttributes,					// 文件属性
	HANDLE hTemplateFile						// 模板文件句柄
	) = CreateFile;

extern "C" __declspec(dllexport)HANDLE WINAPI NewCreateFile(
	LPCTSTR lpFileName,							// 文件名
	DWORD dwDesiredAccess,						// 访问模式
	DWORD dwShareMode,							// 共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// 安全属性(也即销毁方式)
	DWORD dwCreationDisposition,				// how to create
	DWORD dwFlagsAndAttributes,					// 文件属性
	HANDLE hTemplateFile						// 模板文件句柄
) {
	printf("\n\n****************************************\n");
	printf("CreateFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);

	std::cout << "lpFileName: " << pchar << std::endl;
	std::cout << "dwDesiredAccess: 0x" << std::hex << dwDesiredAccess << std::endl;
	std::cout << "dwShareMode: 0x" << std::hex << dwShareMode << std::endl;
	std::cout << "lpSecurityAttributes: 0x" << std::hex << lpSecurityAttributes << std::endl;
	std::cout << "dwCreationDisposition: 0x" << std::hex << dwCreationDisposition << std::endl;
	std::cout << "dwFlagsAndAttributes: 0x" << std::hex << dwFlagsAndAttributes << std::endl;
	std::cout << "hTemplateFile: 0x" << std::hex << hTemplateFile << std::endl;
	
	printf("****************************************\n\n");
	return OldCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// 堆操作 HeapCreate HeapDestroy HeapAlloc HeapFree
static HANDLE(WINAPI* OldHeapCreate)(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) = HeapCreate;

extern "C" __declspec(dllexport)HANDLE WINAPI NewHeapCreate(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) {
	HANDLE hHeap = OldHeapCreate(fIOoptions, dwInitialSize, dwMaximumSize);
	printf("\n\n****************************************\n");
	printf("HeapCreate Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "fIOoptions: " << fIOoptions << std::endl;
	std::cout << "dwInitialSize: " << dwInitialSize << std::endl;
	std::cout << "hHeap: " << hHeap << std::endl;

	printf("****************************************\n\n");
	return hHeap;
}

static BOOL(WINAPI* OldHeapDestroy)(HANDLE) = HeapDestroy;

extern "C" __declspec(dllexport)BOOL WINAPI NewHeapDestroy(HANDLE hHeap) {
	printf("\n\n****************************************\n");
	printf("HeapDestroy Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;

	printf("****************************************\n\n");
	return OldHeapDestroy(hHeap);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
		DetourAttach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
		DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourAttach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourAttach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OldMessageBoxW, NewMessageBoxW);
		DetourDetach(&(PVOID&)OldMessageBoxA, NewMessageBoxA);
		DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourDetach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourDetach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourTransactionCommit();
		break;
	}
	return TRUE;
}