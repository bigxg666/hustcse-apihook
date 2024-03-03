#include "pch.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <dbghelp.h> 
#include <detours.h>

#pragma comment(lib, "detours.lib")

// 函数指针类型，用于保存原始的 memcpy 函数地址
typedef void* (__cdecl* MEMCPY)(void*, const void*, size_t);

// 声明原始的 memcpy 函数指针
MEMCPY pOrigMemcpy = nullptr;

// 自定义的 Hooked memcpy 函数
void* __cdecl HookedMemcpy(void* dest, const void* src, size_t count) {
    // 调用原始的 memcpy 函数
    void* result = pOrigMemcpy(dest, src, count);

    // 记录拷贝的信息
    std::ofstream logfile("memcpy_log.txt", std::ios::app);
    if (logfile.is_open()) {
        // 记录源地址、目的地址和拷贝的字节数
        logfile << "Source Address: " << std::hex << std::showbase << reinterpret_cast<uintptr_t>(src) << std::endl;
        logfile << "Destination Address: " << std::hex << std::showbase << reinterpret_cast<uintptr_t>(dest) << std::endl;
        logfile << "Number of Bytes: " << std::dec << count << std::endl;

        // 记录拷贝的数据内容
        const unsigned char* srcPtr = static_cast<const unsigned char*>(src);
        const unsigned char* destPtr = static_cast<const unsigned char*>(dest);
        logfile << "Copied Data: ";
        for (size_t i = 0; i < count; ++i) {
            logfile << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(srcPtr[i]) << " ";
        }
        logfile << std::endl;

        // 记录拷贝发生的上下文信息，例如调用栈信息
        logfile << "Call Stack: " << std::endl;
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        CONTEXT context;
        RtlCaptureContext(&context);
        STACKFRAME64 stackFrame;
        memset(&stackFrame, 0, sizeof(STACKFRAME64));
        DWORD imageType;
#ifdef _M_IX86
        imageType = IMAGE_FILE_MACHINE_I386;
        stackFrame.AddrPC.Offset = context.Eip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Ebp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Esp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
        imageType = IMAGE_FILE_MACHINE_AMD64;
        stackFrame.AddrPC.Offset = context.Rip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Rsp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#endif
        while (StackWalk64(imageType, process, GetCurrentThread(), &stackFrame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            DWORD64 address = stackFrame.AddrPC.Offset;
            if (address == 0) {
                break;
            }
            DWORD64 moduleBase = SymGetModuleBase64(process, address);
            DWORD64 symbolDisplacement = 0;
            BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + 256];
            PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)symbolBuffer;
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
            symbol->MaxNameLength = 255;
            if (SymGetSymFromAddr64(process, address, &symbolDisplacement, symbol)) {
                logfile << "  " << symbol->Name << " (0x" << std::hex << address << " + 0x" << symbolDisplacement << ")" << std::endl;
            }
            else {
                logfile << "  (No symbol found) (0x" << std::hex << address << ")" << std::endl;
            }
        }

        logfile << "----------------------------------------" << std::endl;
        logfile.close();
    }
    else {
        std::cerr << "Failed to open log file." << std::endl;
    }

    return result;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        // 获取原始的 memcpy 函数地址
        pOrigMemcpy = reinterpret_cast<MEMCPY>(GetProcAddress(GetModuleHandle(TEXT("msvcrt.dll")), "memcpy"));
        if (pOrigMemcpy == nullptr) {
            std::cerr << "Failed to get address of memcpy." << std::endl;
            return FALSE;
        }

        // Hook memcpy 函数
        if (DetourTransactionBegin() != NO_ERROR ||
            DetourUpdateThread(GetCurrentThread()) != NO_ERROR ||
            DetourAttach(reinterpret_cast<void**>(&pOrigMemcpy), HookedMemcpy) != NO_ERROR ||
            DetourTransactionCommit() != NO_ERROR) {
            std::cerr << "Failed to hook memcpy function." << std::endl;
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        // 卸载 Hook
        if (DetourTransactionBegin() != NO_ERROR ||
            DetourUpdateThread(GetCurrentThread()) != NO_ERROR ||
            DetourDetach(reinterpret_cast<void**>(&pOrigMemcpy), HookedMemcpy) != NO_ERROR ||
            DetourTransactionCommit() != NO_ERROR) {
            std::cerr << "Failed to detach from memcpy function." << std::endl;
            return FALSE;
        }
        break;
    }

    return TRUE;
}