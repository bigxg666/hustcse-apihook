
#include "pch.h"
#include "framework.h"
#include "detours.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib,"ws2_32.lib")

SYSTEMTIME st;

// 保存原始函数指针
static SOCKET(WINAPI* OldSocket)(int af, int type, int protocol) = socket;
static int (WINAPI* OldBind)(SOCKET s, const struct sockaddr* name, int namelen) = bind;
static int (WINAPI* OldSend)(SOCKET s, const char* buf, int len, int flags) = send;
static int (WINAPI* OldConnect)(SOCKET s, const struct sockaddr* name, int namelen) = connect;
static int (WINAPI* OldRecv)(SOCKET s, char* buf, int len, int flags) = recv;

// 钩子函数声明
extern "C" __declspec(dllexport) SOCKET WINAPI NewSocket(int af, int type, int protocol);
extern "C" __declspec(dllexport) int WINAPI NewBind(SOCKET s, const struct sockaddr* name, int namelen);
extern "C" __declspec(dllexport) int WINAPI NewSend(SOCKET s, const char* buf, int len, int flags);
extern "C" __declspec(dllexport) int WINAPI NewConnect(SOCKET s, const struct sockaddr* name, int namelen);
extern "C" __declspec(dllexport) int WINAPI NewRecv(SOCKET s, char* buf, int len, int flags);

// 钩子函数定义
extern "C" __declspec(dllexport) SOCKET WINAPI NewSocket(int af, int type, int protocol) {
    // 调用原始函数
    SOCKET result = OldSocket(af, type, protocol);

    
    printf("\n\n****************************************\n");
    printf("socket Hooked\n");
    printf("Socket Parameters:\n");
    printf("Address Family: %d\n", af);
    printf("Socket Type: %d\n", type);
    printf("Protocol: %d\n", protocol);
    printf("****************************************\n\n");

    return result;
}

extern "C" __declspec(dllexport) int WINAPI NewBind(SOCKET s, const struct sockaddr* name, int namelen) {
    // 调用原始函数
    int result = OldBind(s, name, namelen);

    
    printf("\n\n****************************************\n");
    printf("bind Hooked\n");
    printf("Socket Descriptor: %d\n", s);

    // 解析IP和端口
    if (name->sa_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
        char ip[INET_ADDRSTRLEN];
        const char* ip_str = inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
        if (ip_str != NULL) {
            printf("IP Address: %s\n", ip_str);
        }
        printf("Port: %d\n", ntohs(addr_in->sin_port));
    }
    else if (name->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
        char ip[INET6_ADDRSTRLEN];
        const char* ip_str = inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
        if (ip_str != NULL) {
            printf("IP Address: %s\n", ip_str);
        }
        printf("Port: %d\n", ntohs(addr_in6->sin6_port));
    }

    printf("****************************************\n\n");

    return result;
}

extern "C" __declspec(dllexport) int WINAPI NewSend(SOCKET s, const char* buf, int len, int flags) {
    // 调用原始函数
    int result = OldSend(s, buf, len, flags);

    
    printf("\n\n****************************************\n");
    printf("send Hooked\n");
    printf("Socket Descriptor: %d\n", s);
    printf("Data Length: %d\n", len);
    printf("Data: %s\n", buf);
    printf("****************************************\n\n");

    return result;
}

extern "C" __declspec(dllexport) int WINAPI NewConnect(SOCKET s, const struct sockaddr* name, int namelen) {
    // 调用原始函数
    int result = OldConnect(s, name, namelen);

    
    printf("\n\n****************************************\n");
    printf("connect Hooked\n");
    printf("Socket Descriptor: %d\n", s);

    // 解析目标IP和端口
    if (name->sa_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
        char ip[INET_ADDRSTRLEN];
        const char* ip_str = inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
        if (ip_str != NULL) {
            printf("Target IP Address: %s\n", ip_str);
        }
        printf("Target Port: %d\n", ntohs(addr_in->sin_port));
    }
    else if (name->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
        char ip[INET6_ADDRSTRLEN];
        const char* ip_str = inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
        if (ip_str != NULL) {
            printf("Target IP Address: %s\n", ip_str);
        }
        printf("Target Port: %d\n", ntohs(addr_in6->sin6_port));
    }

    printf("****************************************\n\n");

    return result;
}

extern "C" __declspec(dllexport) int WINAPI NewRecv(SOCKET s, char* buf, int len, int flags) {
    // 调用原始函数
    int result = OldRecv(s, buf, len, flags);

    
    printf("\n\n****************************************\n");
    printf("recv Hooked\n");
    printf("Socket Descriptor: %d\n", s);
    printf("Received Data Length: %d\n", result);
    printf("Received Data: %s\n", buf);
    printf("****************************************\n\n");

    return result;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OldSocket, NewSocket);
        DetourAttach(&(PVOID&)OldBind, NewBind);
        DetourAttach(&(PVOID&)OldSend, NewSend);
        DetourAttach(&(PVOID&)OldConnect, NewConnect);
        DetourAttach(&(PVOID&)OldRecv, NewRecv);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)OldSocket, NewSocket);
        DetourDetach(&(PVOID&)OldBind, NewBind);
        DetourDetach(&(PVOID&)OldSend, NewSend);
        DetourDetach(&(PVOID&)OldConnect, NewConnect);
        DetourDetach(&(PVOID&)OldRecv, NewRecv);
		DetourTransactionCommit();
		break;
	}
	return TRUE;
}

