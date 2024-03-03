#include "pch.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <detours.h>

#pragma comment(lib, "detours.lib")

static void* (WINAPI* OldMemcpy)(void* dest, const void* src, size_t count) = memcpy;

// 自定义的内存拷贝函数
extern "C" __declspec(dllexport) void* WINAPI NewMemcpy(void* dest, const void* src, size_t count) {

	void* result = OldMemcpy(dest, src, count);
	// 输出拷贝信息
	std::cout << "Memory copy: " << count << " bytes copied from " << src << " to " << dest << std::endl;

	return result;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OldMemcpy, NewMemcpy);
		DetourTransactionCommit();
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH: {

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OldMemcpy, NewMemcpy);
		DetourTransactionCommit();
		break;
	}
	}
	return TRUE;
}