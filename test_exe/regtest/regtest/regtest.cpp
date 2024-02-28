#define _CRT_SECURE_NO_WARNINGS //可以使用scanf

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <Fileapi.h>

int main() {
	HKEY hk = NULL;
	HKEY hk2 = NULL;
	LPCSTR keyname = "SOFTWARE\\HUST\\HA";
	auto dwType = REG_SZ;
	char buffer[257] = { 0 };
	DWORD dwLen = 256;

	printf("创建注册表项\n");
	auto res = RegCreateKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, NULL);

	printf("关闭注册表项\n");
	RegCloseKey(hk);

	printf("打开注册表项\n");
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, KEY_ALL_ACCESS, &hk2);

	printf("修改注册表项的值\n");
	RegSetValueExA(hk2, "Hello", 0, REG_SZ, (PBYTE)"Hello HUST", 11);

	printf("查询注册表项的值\n");
	RegQueryValueExA(hk2, "Hello", NULL, &dwType, (BYTE*)buffer, &dwLen);

	printf("关闭注册表项\n");
	RegCloseKey(hk2);

	//启动项
	keyname = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	dwType = REG_SZ;
	dwLen = 256;

	printf("在注册表中创建开机启动项\n");
	res = RegCreateKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, NULL);

	printf("关闭注册表项\n");
	RegCloseKey(hk);

	printf("打开注册表项\n");
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, KEY_ALL_ACCESS, &hk2);

	printf("修改注册表项的值\n");
	RegSetValueExA(hk2, "Hello", 0, REG_SZ, (PBYTE)"Hello HUST", 11);

	printf("关闭注册表项\n");
	RegCloseKey(hk2);

	return 0;
}


