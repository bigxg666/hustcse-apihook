// networktest2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 4096

int main() {
	// 初始化 DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 创建套接字
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 向服务器发起请求
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  // 每个字节都用0填充
	sockAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(sockAddr.sin_addr));
	sockAddr.sin_port = htons(1234);
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	Sleep(500);

	// 打开文件并读取内容
	HANDLE hFile = CreateFile(L"a.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytesRead = 0;
		char buffer[BUF_SIZE];
		std::string fileContent;
		while (ReadFile(hFile, buffer, BUF_SIZE, &bytesRead, NULL) && bytesRead > 0) {
			fileContent.append(buffer, bytesRead);
		}
		CloseHandle(hFile);
		// 将文件内容发送到服务器
		send(sock, fileContent.c_str(), fileContent.size(), NULL);
	}
	else {
		DWORD dwError = GetLastError();
		std::cout << "Failed to open file. Error code: " << dwError << std::endl;
	}

	Sleep(500);
	// 接收服务器传回的数据
	char szBuffer[MAXBYTE] = { 0 };
	recv(sock, szBuffer, MAXBYTE, NULL);
	// 输出接收到的数据
	printf("Message from server: %s\n", szBuffer);
	Sleep(8000);
	// 关闭套接字
	closesocket(sock);
	// 终止使用 DLL
	WSACleanup();

	return 0;
}

