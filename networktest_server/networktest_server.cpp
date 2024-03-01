// networktest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
	//初始化 DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//创建套接字
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//绑定套接字
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
	sockAddr.sin_family = AF_INET;  //使用IPv4地址
	inet_pton(AF_INET, "127.0.0.1", &(sockAddr.sin_addr));  //具体的IP地址
	sockAddr.sin_port = htons(1234);  //端口
	bind(serverSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	//进入监听状态
	printf("Now start listening!\n");
	listen(serverSock, 20);
	//接收客户端请求
	SOCKADDR clientAddr;
	int nSize = sizeof(SOCKADDR);
	SOCKET clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &nSize);
	//向客户端发送数据
	char str[32] = "Hello World!";
	send(clientSock, str, strlen(str) + sizeof(char), NULL);
	Sleep(8000);
	//关闭套接字
	closesocket(clientSock);
	closesocket(serverSock);
	//终止 DLL 的使用
	WSACleanup();
}


