
#include "pch.h"
#include "framework.h"
#include "detours.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "detours.lib")

SYSTEMTIME st;


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
	printf("HeapCreate Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;

	printf("****************************************\n\n");
	return OldHeapDestroy(hHeap);
}

static LPVOID(WINAPI* OldHeapAlloc)(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) = HeapAlloc;

extern "C" __declspec(dllexport)LPVOID WINAPI NewHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	LPVOID lpMemory = OldHeapAlloc(hHeap, dwFlags, dwBytes);
	printf("\n\n****************************************\n");
	printf("HeapAlloc Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;
	std::cout << "dwFlags: " << dwFlags << std::endl;
	std::cout << "dwBytes: " << dwBytes << std::endl;
	std::cout << "lpMemory: " << lpMemory << std::endl;

	printf("****************************************\n\n");
	return lpMemory;
}

static BOOL(WINAPI* OldHeapFree)(HANDLE hHeap, DWORD dwFlags, LPVOID lpMemory) = HeapFree;

extern "C" __declspec(dllexport)BOOL WINAPI NewHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMemory) {
	printf("\n\n****************************************\n");
	printf("HeapFree Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;
	std::cout << "dwFlags: " << dwFlags << std::endl;
	std::cout << "lpMemory: " << lpMemory << std::endl;

	printf("****************************************\n\n");
	return OldHeapFree(hHeap, dwFlags, lpMemory);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourAttach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourAttach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
		DetourAttach(&(PVOID&)OldHeapFree, NewHeapFree);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourDetach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourDetach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
		DetourDetach(&(PVOID&)OldHeapFree, NewHeapFree);
		DetourTransactionCommit();
		break;
	}
	return TRUE;
}

