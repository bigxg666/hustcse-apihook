/*
* dllmain.cpp : 定义 DLL 应用程序的入口点。
*/

#include "pch.h"
#include "framework.h"
#include "detours.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib,"ws2_32.lib")

const BYTE LOG_UNINIT = 0xFF;

const BYTE LOG_INFO = 0x00;
const BYTE LOG_CUSTOM = 0x01;
const BYTE LOG_WARNING = 0x02;
const BYTE LOG_ERROR = 0x03;

const BYTE CTRL_QUIT = 0x10;

const int MAX_MSG_NUM = 16;				// 同一时刻最多提供16条提示信息
const int MAX_MSG_LEN = 512;			// 消息的最大长度

const int MAX_FUNCNAME_LEN = 128;		// 函数名最大支持128字节
const int MAX_ARG_NUM = 16;				// 最多支持16个参数
const int MAX_ARGNAME_LEN = 128;		// 每个参数的名称最大支持128字节
const int MAX_ARGDATA_LEN = 512;		// 每个参数的数据最大支持512字节

typedef struct SharedData {
	bool updateFlag = false;			// 更新标志

	SYSTEMTIME st;						// 系统时间
	BYTE type = 0xFF;					// 日志类型

	BYTE msgNum = 0;										// 同一时刻提供的提示信息数量
	char message[MAX_MSG_NUM][MAX_MSG_LEN] = {};			// 提示信息

	// 以下内容仅在type为LOG_CUSTOM时有效
	char funcName[MAX_FUNCNAME_LEN] = {};					// Hook到的函数名
	BYTE argNum = 0;										// 参数个数
	char argName[MAX_ARG_NUM][MAX_ARGNAME_LEN] = {};		// 参数名称表
	char argData[MAX_ARG_NUM][MAX_ARGDATA_LEN] = {};		// 参数数据表
}SharedData;

std::mutex mtx;							// 信号量
SYSTEMTIME st;

// 创建共享内存对象
HANDLE hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), L"MySharedMemory");

// 将共享内存映射到当前进程的地址空间
LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);

// 使用指针pData指向共享内存区域
SharedData* pData = static_cast<SharedData*>(pSharedMemory);

/*
* 实现文件操作监控
* OpenFile CreateFile ReadFile WriteFile
*/

// 创建文件
static HANDLE(WINAPI* OldCreateFile)(
	LPCTSTR lpFileName,							// 文件名
	DWORD dwDesiredAccess,					    // 访问模式
	DWORD dwShareMode,							// 共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// 安全属性（即销毁方式）
	DWORD dwCreationDisposition,				// how to create
	DWORD dwFlagsAndAttributes,					// 文件属性
	HANDLE hTemplateFile						// 模板文件句柄
	) = CreateFile;

extern "C" __declspec(dllexport) HANDLE WINAPI NewCreateFile(
	LPCTSTR lpFileName,							// 文件名
	DWORD dwDesiredAccess,					    // 访问模式
	DWORD dwShareMode,							// 共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// 安全属性（即销毁方式）
	DWORD dwCreationDisposition,				// how to create
	DWORD dwFlagsAndAttributes,					// 文件属性
	HANDLE hTemplateFile						// 模板文件句柄
) {
	printf("\n\n******************************************\n");
	printf("CreateFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);
	delete[] pchar;								// 注意释放内存，否则造成内存泄漏！

	std::cout << "lpFileName: " << std::endl;
	std::cout << "dwDesiredAccess: 0x" << std::hex << dwDesiredAccess << std::endl;
	std::cout << "dwShareMode: 0x" << std::hex << dwShareMode << std::endl;
	std::cout << "lpSecurityAttributes: 0x" << std::hex << lpSecurityAttributes << std::endl;
	std::cout << "dwCreationDisposition: 0x" << std::hex << dwCreationDisposition << std::endl;
	std::cout << "dwFlagsAndAttributes: 0x" << std::hex << dwFlagsAndAttributes << std::endl;
	std::cout << "hTemplateFile: 0x" << std::hex << hTemplateFile << std::endl;

	printf("******************************************\n\n");
	return OldCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// 关闭文件句柄
static BOOL(WINAPI* OldCloseHandle)(HANDLE hObject) = CloseHandle;

extern "C" __declspec(dllexport) BOOL WINAPI NewCloseHandle(HANDLE hObject) {
	
	/*
	* 注意先使用GetFileType获取句柄的类型
	* 然后再使用OldCloseHandle关闭句柄
	* 关闭句柄之后就不能获取句柄类型了
	*/
	DWORD fileType = GetFileType(hObject);
	BOOL result = OldCloseHandle(hObject);

	/*
	* CloseHandle函数可以关闭多种句柄，如文件、进程、管道等
	* 建议增加条件判断，当关闭的句柄为磁盘文件类型时，再输出相关信息
	* 否则会因为输出的无用信息过多，干扰判断
	*/ 
	if (fileType == FILE_TYPE_DISK) {
		printf("\n\n******************************************\n");
		printf("Close DiskFile Handle Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		std::cout << "Handle being closed: 0x" << hObject << std::endl;

		if (result) {
			std::cout << "Handle closed successfully" << std::endl;
		}
		else {
			std::cout << "Failed to close handle. Error code: " << GetLastError() << std::endl;
		}

		printf("******************************************\n\n");
	}
	return result;
}

// 读文件
static BOOL(WINAPI* OldReadFile)(
	HANDLE       hFile,                         // 文件句柄 
	LPVOID       lpBuffer,                      // 读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          // 要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
	) = ReadFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewReadFile(
	HANDLE       hFile,                         // 文件句柄 
	LPVOID       lpBuffer,                      // 读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          // 要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
) {
	/*
	* 增加条件判断，使得仅监测磁盘文件的读取
	* 否则将会出现大量无用信息
	*/ 
	if (GetFileType(hFile) == FILE_TYPE_DISK) {
		printf("\n\n******************************************\n");
		printf("ReadFile Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		// 不建议使用该段代码，该段代码会引起CreateFile操作，造成干扰
		/*
		TCHAR szFileName[MAX_PATH];
		if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0)
		{
			std::cout << "File being read: " << szFileName << std::endl;
		}
		*/

		std::cout << "hFile: " << hFile << std::endl;
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to read: " << nNumberOfBytesToRead << std::endl;
		std::cout << "Number of bytes read (out parameter): 0x" << lpNumberOfBytesRead << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

		printf("******************************************\n\n");
	}
	
	return OldReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

// 写文件
// WriteFile函数的说明：https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-writefile
static BOOL(WINAPI* OldWriteFile)(
	HANDLE       hFile,                         // 文件或I/O设备的句柄（例如文件、文件流、物理磁盘、卷、控制台缓冲区、磁带驱动器、套接字、通信资源、mailslot或管道）
	LPCVOID      lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
	) = WriteFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewWriteFile(
	HANDLE       hFile,                         // 文件或I/O设备的句柄
	LPCVOID      lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
) {
	// 必须添加条件判断，否则无法运行
	if (GetFileType(hFile) == FILE_TYPE_DISK) {
		/*
		* GetFileType(hFile)函数会返回一个标识符，说明hFile句柄所代表的文件类型。
		* 因此条件判断的意思是确保只拦截对磁盘文件的写操作。
		*/
		printf("\n\n******************************************\n");
		printf("WriteFile Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		// 不建议使用该段代码，该段代码会引起CreateFile操作
		/*
		TCHAR szFileName[MAX_PATH];
		if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0) {
			std::cout << "File being written to: " << szFileName << std::endl;
		}
		*/

		std::cout << "hFile: " << hFile << std::endl;
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to write: " << nNumberOfBytesToWrite << std::endl;
		std::cout << "Number of bytes written (out parameter): 0x" << lpNumberOfBytesWritten << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

		printf("******************************************\n\n");
	}
	return OldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

// 删除文件
static BOOL(WINAPI* OldDeleteFile)(LPCTSTR lpFileName) = DeleteFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewDeleteFile(LPCTSTR lpFileName)
{
	printf("\n\n******************************************\n");
	printf("DeleteFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "File being deleted: " << lpFileName << std::endl;

	BOOL result = OldDeleteFile(lpFileName);
	if (result) {
		std::cout << "File deleted successfully" << std::endl;
	}
	else {
		std::cout << "Failed to delete file. Error code: " << GetLastError() << std::endl;
	}

	printf("******************************************\n\n");
	return result;
}



/*
* 实现堆操作监控
* HeapCreate HeapDestroy HeapAlloc HeapFree
*/

// 由于程序alloc过于频繁，我们需要记录特定的堆句柄
// 这一步是为了仅输出对应于我们create句柄的alloc
std::unordered_set<HANDLE> recordedHeapHandles;
// 记录堆句柄的函数
void RecordHeapHandle(HANDLE hHeap)
{
	recordedHeapHandles.insert(hHeap);
}
static HANDLE(WINAPI* OldHeapCreate)(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) = HeapCreate;

extern "C" __declspec(dllexport)HANDLE WINAPI NewHeapCreate(DWORD fIOoptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) {
	HANDLE hHeap = OldHeapCreate(fIOoptions, dwInitialSize, dwMaximumSize);
	printf("\n\n****************************************\n");
	printf("HeapCreate Hooked\n");
	// 记录堆句柄
	RecordHeapHandle(hHeap);
	printf("HeapCreate Hooked [Heap Handle: 0x%p]\n", hHeap);

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

static LPVOID(WINAPI* OldHeapAlloc)(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) = HeapAlloc;

bool isHookEnabled = true;

extern "C" __declspec(dllexport)LPVOID WINAPI NewHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {

	// 检查标志变量
	if (!isHookEnabled) {
		return OldHeapAlloc(hHeap, dwFlags, dwBytes);
	}

	// 禁用钩子函数，“上锁”
	isHookEnabled = false;

	// 检查句柄是否在记录的堆句柄集合中
	if (recordedHeapHandles.find(hHeap) != recordedHeapHandles.end())
	{
		printf("\n\n****************************************\n");
		printf("HeapAlloc Hooked\n");
		// 输出分配内存的打印提示
		printf("HeapAlloc Hooked [Heap Handle: 0x%p]\n", hHeap);
	}
	else {
		isHookEnabled = true;
		return OldHeapAlloc(hHeap, dwFlags, dwBytes);
	}

	LPVOID lpMemory = OldHeapAlloc(hHeap, dwFlags, dwBytes);



	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;
	std::cout << "dwFlags: " << dwFlags << std::endl;
	std::cout << "dwBytes: " << dwBytes << std::endl;
	std::cout << "lpMemory: " << lpMemory << std::endl;

	printf("****************************************\n\n");

	// 启用钩子函数，“开锁”
	isHookEnabled = true;
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



/*
* 实现注册表操作监控
* 注册表相关的API函数功能参考：https://learn.microsoft.com/zh-cn/windows/win32/api/winreg/
*/

// 定义和引入需要Hook的函数和替换的函数
static LSTATUS(WINAPI* OldRegCreateKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) = RegCreateKeyExA;
static LSTATUS(WINAPI* OldRegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) = RegOpenKeyExA;
static LSTATUS(WINAPI* OldRegCloseKey)(HKEY hKey) = RegCloseKey;
static LSTATUS(WINAPI* OldRegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lsendData, LPDWORD lpcbData) = RegQueryValueExA;
static LSTATUS(WINAPI* OldRegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lsendData, DWORD cbData) = RegSetValueExA;

// 函数声明
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCloseKey(HKEY hKey);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lsendData, LPDWORD lpcbData);
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE * lsendData, DWORD cbData);

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
	mtx.lock();												// 上锁

	GetLocalTime(&(pData->st));								// 设置日志时间
	pData->type = LOG_CUSTOM;								// 设置日志类型

	strcpy(pData->funcName, "RegCreateKeyExA");				// 设置函数名
	pData->argNum = 9;										// 设置函数参数的个数

	// 复制参数名
	strcpy(pData->argName[0], "hKey");
	strcpy(pData->argName[1], "lpSubKey");
	strcpy(pData->argName[2], "Reserved");
	strcpy(pData->argName[3], "lpClass");
	strcpy(pData->argName[4], "dwOptions");
	strcpy(pData->argName[5], "samDesired");
	strcpy(pData->argName[6], "lpSecurityAttributes");
	strcpy(pData->argName[7], "phkResult");
	strcpy(pData->argName[8], "lpdwDisposition");

	// 复制参数值
	sprintf(pData->argData[0], "Addr: 0x%p", hKey);			// HKEY大小8字节
	sprintf(pData->argData[1], "Value: %s", lpSubKey);
	sprintf(pData->argData[2], "Value: 0x%x", Reserved);	// DWORD大小4字节
	sprintf(pData->argData[3], "Addr: 0x%p", lpClass);
	sprintf(pData->argData[4], "Value: 0x%x", dwOptions);
	sprintf(pData->argData[5], "Value: 0x%x", samDesired);	// REGSAM大小4字节
	sprintf(pData->argData[6], "Addr: 0x%p", lpSecurityAttributes);
	sprintf(pData->argData[7], "Addr: 0x%p", phkResult);
	sprintf(pData->argData[8], "Addr: 0x%p", lpdwDisposition);

	pData->updateFlag = !pData->updateFlag;					// 更新共享变量，使ReadLog程序监测到更新并展示信息

	/*
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
	*/

	// RegOpenKeyExA函数：打开一个已存在的注册表项，返回值为ERROR_SUCCESS表示打开成功。
	// 调用RegOpenKeyExA函数打开指定的注册表项
	auto judge = OldRegOpenKeyExA(hKey, lpSubKey, 0, samDesired, phkResult);

	// RegCreateKeyExA函数：创建指定的注册表项。如果该项已存在，函数将打开它。
	// 调用RegCreateKeyExA函数创建或打开指定的注册表项
	auto status = OldRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

	if (judge != ERROR_SUCCESS || strcmp(lpSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") == 0) {
		GetLocalTime(&(pData->st));
		pData->type = LOG_WARNING;
		pData->msgNum = 0;
		if (judge != ERROR_SUCCESS) {
			// 返回值不为ERROR_SUCCESS，说明指定的注册表项在函数调用的时候并不存在，因此认为程序存在新增注册表项操作
			printf("此程序存在新增注册表项操作！\n");
			sprintf(pData->message[pData->msgNum++], "此程序存在新增注册表项操作！");
		}

		/*
		* SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run是一个特殊的注册表项。
		* 这个注册表项包含了一系列的键值对，每一个键值对都对应一个需要在系统启动时自动运行的程序。
		*/
		if (strcmp(lpSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") == 0) {
			// 对比打开的子键名称。若子键为"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"，则说明程序很有可能要修改自启动项。
			printf("此程序存在修改注册表自启动项的操作！\n");
			sprintf(pData->message[pData->msgNum++], "此程序存在修改注册表自启动项的操作！");
		}
		
		pData->updateFlag = !pData->updateFlag;
	}

	mtx.unlock();
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
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "RegOpenKeyExA");
	pData->argNum = 5;

	strcpy(pData->argName[0], "hKey");
	strcpy(pData->argName[1], "lpSubKey");
	strcpy(pData->argName[2], "ulOptions");
	strcpy(pData->argName[3], "samDesired");
	strcpy(pData->argName[4], "phkResult");

	sprintf(pData->argData[0], "Addr: 0x%p", hKey);
	sprintf(pData->argData[1], "Value: %s", lpSubKey);
	sprintf(pData->argData[2], "Value: 0x%x", ulOptions);
	sprintf(pData->argData[3], "Value: 0x%x", samDesired);
	sprintf(pData->argData[4], "Addr: 0x%p", phkResult);

	pData->updateFlag = !pData->updateFlag;

	/*
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpSubKey: " << lpSubKey << std::endl;
	std::cout << "ulOptions: " << ulOptions << std::endl;
	std::cout << "samDesired: " << samDesired << std::endl;
	std::cout << "phkResult: " << phkResult << std::endl;
	*/

	mtx.unlock();
	return OldRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

// 关闭注册表的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegCloseKey(
	HKEY hKey				// 注册表主键的句柄
) {
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "RegCloseKey");
	pData->argNum = 1;

	strcpy(pData->argName[0], "hKey");

	sprintf(pData->argData[0], "Addr: 0x%p", hKey);

	pData->updateFlag = !pData->updateFlag;

	/*
	std::cout << "hKey: " << hKey << std::endl;
	*/

	mtx.unlock();
	return OldRegCloseKey(hKey);
}

// 查询注册表键值的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegQueryValueExA(
	HKEY hKey,				// 注册表主键的句柄
	LPCSTR lpValueName,		// 值的名称
	LPDWORD lpReserved,		// 保留，必须为NULL
	LPDWORD lpType,			// 接受值类型的指针
	LPBYTE lsendData,		// 接收值数据的缓冲区的指针
	LPDWORD lpcbData		// 缓冲区的大小，以字节为单位
) {
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "RegQueryValueExA");
	pData->argNum = 6;

	strcpy(pData->argName[0], "hKey");
	strcpy(pData->argName[1], "lpValueName");
	strcpy(pData->argName[2], "lpReserved");
	strcpy(pData->argName[3], "lpType");
	strcpy(pData->argName[4], "lsendData");
	strcpy(pData->argName[5], "lpcbData");

	sprintf(pData->argData[0], "Addr: 0x%p", hKey);
	sprintf(pData->argData[1], "Value: %s", lpValueName);
	sprintf(pData->argData[2], "Addr: 0x%p", lpReserved);
	sprintf(pData->argData[3], "Addr: 0x%p", lpType);
	sprintf(pData->argData[4], "Addr: 0x%p", lsendData);
	sprintf(pData->argData[5], "Addr: 0x%p", lpcbData);

	pData->updateFlag = !pData->updateFlag;

	/*
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "lpReserved: " << lpReserved << std::endl;
	std::cout << "lpType: " << lpType << std::endl;
	std::cout << "lsendData: " << lsendData << std::endl;
	std::cout << "lpcbData: " << lpcbData << std::endl;
	*/

	mtx.unlock();
	return OldRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lsendData, lpcbData);
}

// 修改注册表键值的操作
extern "C" __declspec(dllexport) LSTATUS WINAPI NewRegSetValueExA(
	HKEY hKey,				// 注册表主键的句柄
	LPCSTR lpValueName,		// 值的名称
	DWORD Reserved,			// 保留，必须为0
	DWORD dwType,			// 值的类型
	const BYTE* lsendData,	// 指向包含值数据的缓冲区的指针
	DWORD cbData			// 缓冲区的大小，以字节为单位
) {
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "RegSetValueExA");
	pData->argNum = 6;

	strcpy(pData->argName[0], "hKey");
	strcpy(pData->argName[1], "lpValueName");
	strcpy(pData->argName[2], "Reserved");
	strcpy(pData->argName[3], "dwType");
	strcpy(pData->argName[4], "lsendData");
	strcpy(pData->argName[5], "cbData");

	sprintf(pData->argData[0], "Addr: 0x%p", hKey);
	sprintf(pData->argData[1], "Value: %s", lpValueName);
	sprintf(pData->argData[2], "Value: 0x%x", Reserved);
	sprintf(pData->argData[3], "Value: 0x%x", dwType);
	sprintf(pData->argData[4], "Addr: 0x%p", lsendData);
	sprintf(pData->argData[5], "Value: 0x%x", cbData);

	pData->updateFlag = !pData->updateFlag;

	/*
	std::cout << "hKey: " << hKey << std::endl;
	std::cout << "lpValueName: " << lpValueName << std::endl;
	std::cout << "Reserved: " << Reserved << std::endl;
	std::cout << "dwType: " << dwType << std::endl;
	std::cout << "lsendData: " << lsendData << std::endl;
	std::cout << "cbData: " << cbData << std::endl;
	*/

	/*
	* RegSetValueExA函数：
	* 设置注册表项下指定值的数据和类型。
	*/

	// 调用RegSetValueExA函数设置指定的注册表项的值，并将返回的状态保存在status变量中
	auto status = OldRegSetValueExA(hKey, lpValueName, Reserved, dwType, lsendData, cbData);

	// 如果返回值为ERROR_SUCCESS，说明函数成功执行（也就是修改了注册表项的值）
	if (status == ERROR_SUCCESS) {
		GetLocalTime(&(pData->st));
		pData->type = LOG_WARNING;
		pData->msgNum = 0;
		printf("此程序存在修改注册表项的操作！\n");
		sprintf(pData->message[pData->msgNum++], "此程序存在修改注册表项的操作！");
		pData->updateFlag = !pData->updateFlag;
	}

	mtx.unlock();
	return status;
}

//网络部分api的hook
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

// 主函数
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		if (hSharedMemory == NULL) {
			printf("Failed to create shared memory.\n");
			return EXIT_FAILURE;
		}

		if (pSharedMemory == NULL) {
			printf("Failed to map shared memory.\n");
			CloseHandle(hSharedMemory);
			return EXIT_FAILURE;
		}

		GetLocalTime(&(pData->st));
		pData->type = LOG_INFO;
		pData->msgNum = 0;
		sprintf(pData->message[pData->msgNum++], "行为监测开始。");
		pData->updateFlag = !pData->updateFlag;

		printf("行为监测开始。");

		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		// 文件操作
		DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourAttach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourAttach(&(PVOID&)OldReadFile, NewReadFile);
		DetourAttach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourAttach(&(PVOID&)OldDeleteFile, NewDeleteFile);

		// 堆操作
		DetourAttach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourAttach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourAttach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
		DetourAttach(&(PVOID&)OldHeapFree, NewHeapFree);

		// 注册表操作
		DetourAttach(&(PVOID&)OldRegCreateKeyExA, NewRegCreateKeyExA);
		DetourAttach(&(PVOID&)OldRegOpenKeyExA, NewRegOpenKeyExA);
		DetourAttach(&(PVOID&)OldRegQueryValueExA, NewRegQueryValueExA);
		DetourAttach(&(PVOID&)OldRegSetValueExA, NewRegSetValueExA);
		DetourAttach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);

		//网络操作
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

		// 文件操作
		DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourDetach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourDetach(&(PVOID&)OldReadFile, NewReadFile);
		DetourDetach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourDetach(&(PVOID&)OldDeleteFile, NewDeleteFile);

		// 堆操作
		DetourDetach(&(PVOID&)OldHeapCreate, NewHeapCreate);
		DetourDetach(&(PVOID&)OldHeapDestroy, NewHeapDestroy);
		DetourDetach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
		DetourDetach(&(PVOID&)OldHeapFree, NewHeapFree);

		// 注册表操作
		DetourDetach(&(PVOID&)OldRegCreateKeyExA, NewRegCreateKeyExA);
		DetourDetach(&(PVOID&)OldRegOpenKeyExA, NewRegOpenKeyExA);
		DetourDetach(&(PVOID&)OldRegQueryValueExA, NewRegQueryValueExA);
		DetourDetach(&(PVOID&)OldRegSetValueExA, NewRegSetValueExA);
		DetourDetach(&(PVOID&)OldRegCloseKey, NewRegCloseKey);

		//网络操作
		DetourDetach(&(PVOID&)OldSocket, NewSocket);
		DetourDetach(&(PVOID&)OldBind, NewBind);
		DetourDetach(&(PVOID&)OldSend, NewSend);
		DetourDetach(&(PVOID&)OldConnect, NewConnect);
		DetourDetach(&(PVOID&)OldRecv, NewRecv);

		DetourTransactionCommit();

		/*
		pData->updateFlag = !pData->updateFlag;
		pData->type = LOG_INFO;
		sprintf(pData->message, "行为监测结束。\n");
		printf("行为监测结束。\n");

		// 解除共享内存的映射
		UnmapViewOfFile(pSharedMemory);

		// 关闭共享内存对象
		CloseHandle(hSharedMemory);
		*/

		break;
	}

	return TRUE;
}
