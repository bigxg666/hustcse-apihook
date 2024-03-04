/*
* dllmain.cpp : 定义 DLL 应用程序的入口点。
*/

#include "pch.h"
#include "framework.h"
#include "detours.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <unordered_set>
#include <mutex>

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "ws2_32.lib")



const BYTE LOG_UNINIT = 0xFF;

const BYTE LOG_INFO = 0x00;
const BYTE LOG_CUSTOM = 0x01;
const BYTE LOG_WARNING = 0x02;
const BYTE LOG_ERROR = 0x03;

const BYTE CTRL_QUIT = 0x10;

const int MAX_CNT_NUM = UINT8_MAX;		// 日志表的表长

const int MAX_MSG_LEN = 512;			// 提示信息的最大长度

const int MAX_FUNCNAME_LEN = 128;		// 函数名最大支持128字节
const int MAX_ARG_NUM = 16;				// 最多支持16个参数
const int MAX_ARGNAME_LEN = 128;		// 每个参数的名称最大支持128字节
const int MAX_ARGDATA_LEN = 512;		// 每个参数的数据最大支持512字节

// 日志信息结构体
typedef struct LogInfo {
	uint8_t cnt = 0;					// 记录在日志表中的下标

	SYSTEMTIME st;						// 系统时间
	BYTE type = 0xFF;					// 日志类型

	char message[MAX_MSG_LEN] = {};		// 提示信息

	// 以下内容仅在type为LOG_CUSTOM时有效
	char funcName[MAX_FUNCNAME_LEN] = {};				// Hook到的函数名
	BYTE argNum = 0;									// 参数个数
	char argName[MAX_ARG_NUM][MAX_ARGNAME_LEN] = {};	// 参数名称表
	char argData[MAX_ARG_NUM][MAX_ARGDATA_LEN] = {};	// 参数数据表
}LogInfo;

// 共享变量结构体
typedef struct SharedData {
	uint8_t cnt = 0;									// 记录日志表中信息的个数
	LogInfo logInfoList[MAX_CNT_NUM] = {};				// 日志表
}SharedData;

// 创建共享内存对象
HANDLE hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), L"MySharedMemory");

// 将共享内存映射到当前进程的地址空间
LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);

// 使用指针pSharedData指向共享内存区域
SharedData* pSharedData = static_cast<SharedData*>(pSharedMemory);

uint8_t cnt = 0;										// 日志表的下标
LogInfo* pData = &(pSharedData->logInfoList[0]);		// 当前日志结构

// 更新共享变量相关信息
// 每输出一条共享日志信息之后都要调用一次该函数
void update() {
	cnt++;
	cnt %= MAX_CNT_NUM;
	pSharedData->cnt = cnt;								// 更新日志表中信息的个数
	pData = &(pSharedData->logInfoList[cnt]);			// 令pData指向下一个日志表项
}



// 文件信息结构体
typedef struct FileData {
	std::string filename;				// 完整文件名
	std::string folderPath;				// 文件夹路径
	std::string fileExtension;			// 文件扩展名
}FileData;
std::set<std::string> accessedFolders;	// 存储已经访问过的文件夹路径

std::unordered_set<std::string> fileContents;// 添加一个全局集合，用于存储已经读取的文件内容

// 获取文件夹路径及文件后缀等信息
FileData GetFileData(HANDLE hFile) {
	FileData result;

	// 会引起CreateFile操作，但在NewCreateFile函数中进行了过滤
	TCHAR szFileName[MAX_PATH];
	if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0) {
		std::wstring wsFileName(szFileName);
		std::string fileName(wsFileName.begin(), wsFileName.end());
		result.filename = fileName;
		// 去除路径前缀 \\?\ 
		if (result.filename.substr(0, 4) == "\\\\?\\") {
			result.filename = result.filename.substr(4);
		}
		
		// 从文件名中提取文件扩展名
		size_t dotPos = result.filename.find_last_of('.');
		if (dotPos != std::string::npos) {
			result.fileExtension = result.filename.substr(dotPos);
		}
		else {
			GetLocalTime(&(pData->st));
			pData->type = LOG_ERROR;
			printf("无法提取文件扩展名！\n");
			sprintf(pData->message, "无法提取文件扩展名！");
			update();
			// std::cout << "Unable to extract the file extension." << std::endl;
		}

		// 获取文件夹路径
		size_t found = result.filename.find_last_of("\\/");
		if (found != std::string::npos) {
			result.folderPath = result.filename.substr(0, found);
			// 将文件夹路径添加到已访问文件夹集合中
			accessedFolders.insert(result.folderPath);
			// 打印访问过的文件夹
			// std::cout << "Folder path: " << result.folderPath << std::endl;
		}
		else {
			GetLocalTime(&(pData->st));
			pData->type = LOG_ERROR;
			printf("无法提取文件夹路径！\n");
			sprintf(pData->message, "无法提取文件夹路径！");
			update();
			// std::cout << "Unable to extract the folder path." << std::endl;
		}
	}
	else {
		GetLocalTime(&(pData->st));
		pData->type = LOG_ERROR;
		printf("无法获取文件名！\n");
		sprintf(pData->message, "无法获取文件名！");
		update();
		// std::cout << "Unable to get the file name." << std::endl;
	}
	return result;
}



// 全局变量定义
std::mutex mtx;							// 信号量
SYSTEMTIME st;							// 系统时间



/*
* 实现文件操作监控
* CreateFile CloseHandle ReadFile WriteFile DeleteFile CopyFile
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
	if (wcsstr(lpFileName, L"log") != nullptr || wcsstr(lpFileName, L".config") != nullptr || wcsstr(lpFileName, L"MountPointManage") != nullptr) {
		// 如果包含需要过滤的字符串，则直接返回原函数
		return OldCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "CreateFile");
	pData->argNum = 7;

	strcpy(pData->argName[0], "lpFileName");
	strcpy(pData->argName[1], "dwDesiredAccess");
	strcpy(pData->argName[2], "dwShareMode");
	strcpy(pData->argName[3], "lpSecurityAttributes");
	strcpy(pData->argName[4], "dwCreationDisposition");
	strcpy(pData->argName[5], "dwFlagsAndAttributes");
	strcpy(pData->argName[6], "hTemplateFile");

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);

	sprintf(pData->argData[0], "Value: %s", pchar);
	sprintf(pData->argData[1], "Value: 0x%x", dwDesiredAccess);
	sprintf(pData->argData[2], "Value: 0x%x", dwShareMode);
	sprintf(pData->argData[3], "Addr: 0x%p", lpSecurityAttributes);
	sprintf(pData->argData[4], "Value: 0x%x", dwCreationDisposition);
	sprintf(pData->argData[5], "Value: 0x%x", dwFlagsAndAttributes);
	sprintf(pData->argData[6], "Addr: 0x%p", hTemplateFile);

	update();

	/*
	printf("\n\n******************************************\n");
	printf("CreateFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

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

	printf("******************************************\n\n");
	*/

	delete[] pchar;
	mtx.unlock();
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
	BOOL result = FALSE;
	/*
	* CloseHandle函数可以关闭多种句柄，如文件、进程、管道等
	* 建议增加条件判断，当关闭的句柄为磁盘文件类型时，再输出相关信息
	* 否则会因为输出的无用信息过多，干扰判断
	*/
	if (fileType == FILE_TYPE_DISK) {
		FileData myFile = GetFileData(hObject);	// 要在关闭句柄之前进行

		mtx.lock();
		GetLocalTime(&(pData->st));
		pData->type = LOG_CUSTOM;

		strcpy(pData->funcName, "CloseHandle");
		pData->argNum = 1;

		strcpy(pData->argName[0], "hObject");

		sprintf(pData->argData[0], "Addr: 0x%p", hObject);

		update();

		/*

		printf("\n\n******************************************\n");
		printf("Close DiskFile Handle Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		std::cout << "DiskFile being closed: " << myFile.filename << std::endl;

		result = OldCloseHandle(hObject);
		if (result) {
			std::cout << "DiskFile closed successfully" << std::endl;
		}
		else {
			std::cout << "Failed to close DiskFile. Error code: " << GetLastError() << std::endl;
		}

		printf("******************************************\n\n");
		*/

		mtx.unlock();
	}

	return result;
}

// 读文件
static BOOL(WINAPI* OldReadFile)(
	HANDLE       hFile,                         // 文件句柄 
	LPVOID       lpBuffer,                      // 读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          // 要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,           // 实际读取到的字节数
	LPOVERLAPPED lpOverlapped                   // OVERLAPPED 结构，一般设定为 NULL 
	) = ReadFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewReadFile(
	HANDLE       hFile,                         // 文件句柄 
	LPVOID       lpBuffer,                      // 读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          // 要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,           // 实际读取到的字节数
	LPOVERLAPPED lpOverlapped                   // OVERLAPPED 结构，一般设定为 NULL 
) {
	BOOL result = OldReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	/*
	* 增加条件判断，使得仅监测磁盘文件的读取
	* 否则将会出现大量无用信息
	*/
	if (GetFileType(hFile) == FILE_TYPE_DISK) {
		FileData myFile = GetFileData(hFile);

		mtx.lock();
		GetLocalTime(&(pData->st));
		pData->type = LOG_CUSTOM;

		strcpy(pData->funcName, "ReadFile");
		pData->argNum = 6;

		// 加一个读取文件的文件名
		strcpy(pData->argName[0], "fileName");
		strcpy(pData->argName[1], "hFile");
		strcpy(pData->argName[2], "lpBuffer");
		strcpy(pData->argName[3], "nNumberOfBytesToRead");
		strcpy(pData->argName[4], "lpNumberOfBytesRead");
		strcpy(pData->argName[5], "lpOverlapped");

		sprintf(pData->argData[0], "Value: %s", myFile.filename.c_str());
		sprintf(pData->argData[1], "Addr: 0x%p", hFile);
		sprintf(pData->argData[2], "Addr: 0x%p", lpBuffer);
		sprintf(pData->argData[3], "Value: 0x%x", nNumberOfBytesToRead);
		sprintf(pData->argData[4], "Value: 0x%x", *lpNumberOfBytesRead);
		sprintf(pData->argData[5], "Addr: 0x%p", lpOverlapped);

		update();

		/*
		printf("\n\n******************************************\n");
		printf("ReadFile Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		*/

		// 不建议使用该段代码，该段代码会引起CreateFile操作，造成干扰
		/*
		TCHAR szFileName[MAX_PATH];
		if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0)
		{
			std::cout << "File being read: " << szFileName << std::endl;
		}
		*/

		/*
		std::cout << "Filename: " << myFile.filename << std::endl;		// 改为了输出文件名
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to read: " << nNumberOfBytesToRead << std::endl;
		std::cout << "Number of bytes read (out parameter): 0x" << lpNumberOfBytesRead << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;
		*/
		
		// 判断操作范围是否有多个文件夹
		if (accessedFolders.size() > 1) {

			GetLocalTime(&(pData->st));
			pData->type = LOG_WARNING;
			printf("此程序存在读取多个文件的操作！\n");
			sprintf(pData->message, "此程序存在读取多个文件的操作！");
			update();

			/*
			// 将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Operation spans multiple folders." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			*/
		}

		// 判断是否成功读取文件内容,以便于判断是否将文件内容读取后发送到网络
		if (result && *lpNumberOfBytesRead > 0)
		{
			// 将读取的内容转换为字符串
			std::string content(static_cast<char*>(lpBuffer), *lpNumberOfBytesRead);
			// 将内容存储到集合中
			fileContents.insert(content);
		}

		// printf("******************************************\n\n");
		
		mtx.unlock();
	}
	return result;
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
		// 不建议使用该段代码，该段代码会引起CreateFile操作    
		/*
		TCHAR szFileName[MAX_PATH];
		if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0) {
			std::cout << "File being written to: " << szFileName << std::endl;
		}
		*/
		FileData myFile = GetFileData(hFile);

		mtx.lock();
		GetLocalTime(&(pData->st));
		pData->type = LOG_CUSTOM;

		strcpy(pData->funcName, "WriteFile");
		pData->argNum = 6;

		// 加一个读取文件的文件名
		strcpy(pData->argName[0], "fileName");
		strcpy(pData->argName[1], "hFile");
		strcpy(pData->argName[2], "lpBuffer");
		strcpy(pData->argName[3], "nNumberOfBytesToRead");
		strcpy(pData->argName[4], "lpNumberOfBytesRead");
		strcpy(pData->argName[5], "lpOverlapped");

		sprintf(pData->argData[0], "Value: %s", myFile.filename.c_str());
		sprintf(pData->argData[1], "Addr: 0x%p", hFile);
		sprintf(pData->argData[2], "Addr: 0x%p", lpBuffer);
		sprintf(pData->argData[3], "Value: 0x%x", nNumberOfBytesToWrite);
		sprintf(pData->argData[4], "Value: 0x%x", *lpNumberOfBytesWritten);
		sprintf(pData->argData[5], "Addr: 0x%p", lpOverlapped);

		update();

		/*
		printf("\n\n******************************************\n");
		printf("WriteFile Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		std::cout << "Filename: " << myFile.filename << std::endl;//改为了输出文件名
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to write: " << nNumberOfBytesToWrite << std::endl;
		std::cout << "Number of bytes written (out parameter): 0x" << lpNumberOfBytesWritten << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

		*/
		// 判断操作范围是否有多个文件夹
		if (accessedFolders.size() > 1) {
			GetLocalTime(&(pData->st));
			pData->type = LOG_WARNING;
			printf("此程序存在写入多个文件的操作！\n");
			sprintf(pData->message, "此程序存在写入多个文件的操作！");
			update();

			/*
			// 将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Operation spans multiple folders." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			*/
		}

		// 通过后缀来判断是否修改了可执行文件包括exe，dll，ocx等
		if (myFile.fileExtension == ".exe" || myFile.fileExtension == ".dll" || myFile.fileExtension == ".ocx") {
			GetLocalTime(&(pData->st));
			pData->type = LOG_WARNING;
			printf("此程序可能存在自我复制的操作！\n");
			sprintf(pData->message, "此程序可能存在自我复制的操作！");
			update();

			/*
			// 将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Possible self-replication." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			*/
		}

		// printf("******************************************\n\n");
		mtx.unlock();
	}

	return OldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

// 删除文件
static BOOL(WINAPI* OldDeleteFile)(LPCTSTR lpFileName) = DeleteFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewDeleteFile(LPCTSTR lpFileName) {

	/*
	printf("\n\n******************************************\n");
	printf("DeleteFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	*/

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "DeleteFile");
	pData->argNum = 1;

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);

	strcpy(pData->argName[0], "lpFileName");

	sprintf(pData->argData[0], "Value: %s", pchar);

	update();

	/*
	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);
	std::cout << "File being deleted: " << pchar << std::endl;
	*/

	GetLocalTime(&(pData->st));
	pData->type = LOG_WARNING;
	printf("此程序进行了删除文件的操作，");
	sprintf(pData->message, "此程序进行了删除文件的操作，");

	BOOL result = OldDeleteFile(lpFileName);
	if (result) {
		printf("且删除成功！\n");
		sprintf(pData->message, "且删除成功！");
		// std::cout << "File deleted successfully" << std::endl;
	}
	else {
		printf("且删除失败！\n");
		sprintf(pData->message, "且删除失败！");
		// std::cout << "Failed to delete file. Error code: " << GetLastError() << std::endl;
	}
	update();

	// printf("******************************************\n\n");
	delete[] pchar;
	mtx.unlock();
	return result;
}

// 监测是否存在自我复制的情况
static BOOL(WINAPI* OldCopyFile)(
	LPCTSTR lpExistingFileName,          // 现有文件名称
	LPCTSTR lpNewFileName,               // 新文件名称
	BOOL beFailIfExist
	) = CopyFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewCopyFile(
	LPCTSTR lpExistingFileName,          // 现有文件名称
	LPCTSTR lpNewFileName,               // 新文件名称
	BOOL beFailIfExist
) {
	BOOL result = OldCopyFile(lpExistingFileName, lpNewFileName, beFailIfExist);

	/*
	printf("\n\n******************************************\n");
	printf("CopyFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	*/

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "CopyFile");
	pData->argNum = 3;

	strcpy(pData->argName[0], "lpExistingFileName");
	strcpy(pData->argName[1], "lpNewFileName");
	strcpy(pData->argName[2], "beFailIfExist");

	int num;
	char* pchar;

	num = WideCharToMultiByte(CP_OEMCP, NULL, lpExistingFileName, -1, NULL, 0, NULL, FALSE);
	pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpExistingFileName, -1, pchar, num, NULL, FALSE);

	sprintf(pData->argData[0], "Value: %s", pchar);

	delete[] pchar;

	num = WideCharToMultiByte(CP_OEMCP, NULL, lpNewFileName, -1, NULL, 0, NULL, FALSE);
	pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpNewFileName, -1, pchar, num, NULL, FALSE);

	sprintf(pData->argData[1], "Value: %s", pchar);

	delete[] pchar;

	sprintf(pData->argData[2], "Value: 0x%x", beFailIfExist);

	update();

	GetLocalTime(&(pData->st));
	pData->type = LOG_WARNING;
	printf("此程序进行了自我复制的操作，");
	sprintf(pData->message, "此程序进行了自我复制的操作，");

	if (result) {
		/*
		// 将输出字体颜色设置为红色
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		std::cout << "Warning: Self-copy detected." << std::endl;
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		*/
		printf("且复制成功！\n");
		sprintf(pData->message, "且复制成功！");
	}
	else {
		// std::cout << "Failed to copy file." << std::endl;
		printf("且复制失败！\n");
		sprintf(pData->message, "且复制失败！");
	}

	// printf("******************************************\n\n");
	mtx.unlock();
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
void RecordHeapHandle(HANDLE hHeap) {
	recordedHeapHandles.insert(hHeap);
}

static HANDLE(WINAPI* OldHeapCreate)(
	DWORD fIOoptions,
	SIZE_T dwInitialSize,
	SIZE_T dwMaximumSize
	) = HeapCreate;

extern "C" __declspec(dllexport)HANDLE WINAPI NewHeapCreate(
	DWORD fIOoptions,
	SIZE_T dwInitialSize,
	SIZE_T dwMaximumSize
) {
	HANDLE hHeap = OldHeapCreate(fIOoptions, dwInitialSize, dwMaximumSize);

	// 记录堆句柄
	RecordHeapHandle(hHeap);

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "HeapCreate");
	pData->argNum = 4;

	// 加一个堆句柄
	strcpy(pData->argName[0], "hHeap");
	strcpy(pData->argName[1], "fIOoptions");
	strcpy(pData->argName[2], "dwInitialSize");
	strcpy(pData->argName[3], "dwMaximumSize");

	sprintf(pData->argData[0], "Addr: 0x%p", hHeap);
	sprintf(pData->argData[1], "Value: 0x%x", fIOoptions);
	sprintf(pData->argData[2], "Value: 0x%llx", dwInitialSize);
	sprintf(pData->argData[3], "Value: 0x%llx", dwMaximumSize);

	update();

	/*
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
	*/

	mtx.unlock();
	return hHeap;
}

static BOOL(WINAPI* OldHeapDestroy)(HANDLE hHeap) = HeapDestroy;

extern "C" __declspec(dllexport)BOOL WINAPI NewHeapDestroy(HANDLE hHeap) {
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "HeapDestroy");
	pData->argNum = 1;

	strcpy(pData->argName[0], "hHeap");

	sprintf(pData->argData[0], "Addr: 0x%p", hHeap);

	update();

	/*
	printf("\n\n****************************************\n");
	printf("HeapDestroy Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;

	printf("****************************************\n\n");
	*/

	mtx.unlock();
	return OldHeapDestroy(hHeap);
}

static LPVOID(WINAPI* OldHeapAlloc)(
	HANDLE hHeap,
	DWORD dwFlags,
	SIZE_T dwBytes
	) = HeapAlloc;

bool isHookEnabled = true;

extern "C" __declspec(dllexport)LPVOID WINAPI NewHeapAlloc(
	HANDLE hHeap,
	DWORD dwFlags,
	SIZE_T dwBytes
) {
	// 检查标志变量
	if (!isHookEnabled) {
		return OldHeapAlloc(hHeap, dwFlags, dwBytes);
	}

	// 禁用钩子函数，“上锁”
	isHookEnabled = false;

	// 检查句柄是否在记录的堆句柄集合中
	if (recordedHeapHandles.find(hHeap) != recordedHeapHandles.end())
	{
		/*
		printf("\n\n****************************************\n");
		printf("HeapAlloc Hooked\n");
		// 输出分配内存的打印提示
		printf("HeapAlloc Hooked [Heap Handle: 0x%p]\n", hHeap);
		*/
	}
	else {
		isHookEnabled = true;
		return OldHeapAlloc(hHeap, dwFlags, dwBytes);
	}

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "HeapAlloc");
	pData->argNum = 3;

	strcpy(pData->argName[0], "hHeap");
	strcpy(pData->argName[1], "dwFlags");
	strcpy(pData->argName[2], "dwBytes");

	sprintf(pData->argData[0], "Addr: 0x%p", hHeap);
	sprintf(pData->argData[1], "Value: 0x%x", dwFlags);
	sprintf(pData->argData[2], "Value: 0x%llx", dwBytes);

	update();

	/*
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;
	std::cout << "dwFlags: " << dwFlags << std::endl;
	std::cout << "dwBytes: " << dwBytes << std::endl;
	std::cout << "lpMemory: " << lpMemory << std::endl;

	printf("****************************************\n\n");
	*/

	mtx.unlock();
	// 启用钩子函数，“开锁”
	isHookEnabled = true;
	return OldHeapAlloc(hHeap, dwFlags, dwBytes);
}

static BOOL(WINAPI* OldHeapFree)(
	HANDLE hHeap,
	DWORD dwFlags,
	LPVOID lpMemory
	) = HeapFree;

extern "C" __declspec(dllexport)BOOL WINAPI NewHeapFree(
	HANDLE hHeap,
	DWORD dwFlags,
	LPVOID lpMemory
) {
	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "HeapFree");
	pData->argNum = 3;

	strcpy(pData->argName[0], "hHeap");
	strcpy(pData->argName[1], "dwFlags");
	strcpy(pData->argName[2], "lpMemory");

	sprintf(pData->argData[0], "Addr: 0x%p", hHeap);
	sprintf(pData->argData[1], "Value: 0x%x", dwFlags);
	sprintf(pData->argData[2], "Addr: 0x%p", lpMemory);

	update();

	/*
	printf("\n\n****************************************\n");
	printf("HeapFree Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出: %d-%d-%d %02d:%02d:%02d:%03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "hHeap: " << hHeap << std::endl;
	std::cout << "dwFlags: " << dwFlags << std::endl;
	std::cout << "lpMemory: " << lpMemory << std::endl;

	printf("****************************************\n\n");
	*/

	mtx.unlock();
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
	sprintf(pData->argData[0], "Addr: 0x%p", hKey);
	sprintf(pData->argData[1], "Value: %s", lpSubKey);
	sprintf(pData->argData[2], "Value: 0x%x", Reserved);
	sprintf(pData->argData[3], "Addr: 0x%p", lpClass);
	sprintf(pData->argData[4], "Value: 0x%x", dwOptions);
	sprintf(pData->argData[5], "Value: 0x%x", samDesired);
	sprintf(pData->argData[6], "Addr: 0x%p", lpSecurityAttributes);
	sprintf(pData->argData[7], "Addr: 0x%p", phkResult);
	sprintf(pData->argData[8], "Addr: 0x%p", lpdwDisposition);

	// 消息输出完毕后一定要调用update函数使全局变量cnt自增，并使pData指针指向下一个表项
	update();

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

	// 返回值不为ERROR_SUCCESS，说明指定的注册表项在函数调用的时候并不存在，因此认为程序存在新增注册表项操作
	auto judge = OldRegOpenKeyExA(hKey, lpSubKey, 0, samDesired, phkResult);
	if (judge != ERROR_SUCCESS) {
		GetLocalTime(&(pData->st));
		pData->type = LOG_WARNING;
		printf("此程序存在新增注册表项操作！\n");
		sprintf(pData->message, "此程序存在新增注册表项操作！");
		update();
	}

	// RegCreateKeyExA函数：创建指定的注册表项。如果该项已存在，函数将打开它。
	// 调用RegCreateKeyExA函数创建或打开指定的注册表项
	auto status = OldRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

	/*
	* SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run是一个特殊的注册表项。
	* 这个注册表项包含了一系列的键值对，每一个键值对都对应一个需要在系统启动时自动运行的程序。
	*/
	// 对比打开的子键名称。若子键为"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"，则说明程序很有可能要修改自启动项。
	if (strcmp(lpSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") == 0) {
		GetLocalTime(&(pData->st));
		pData->type = LOG_WARNING;
		printf("此程序存在修改注册表自启动项的操作！\n");
		sprintf(pData->message, "此程序存在修改注册表自启动项的操作！");
		update();
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

	update();

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

	update();

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

	update();

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
	const BYTE * lsendData,	// 指向包含值数据的缓冲区的指针
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

	update();

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
		printf("此程序存在修改注册表项的操作！\n");
		sprintf(pData->message, "此程序存在修改注册表项的操作！");
		update();
	}

	mtx.unlock();
	return status;
}

// 网络部分api的hook
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
extern "C" __declspec(dllexport) SOCKET WINAPI NewSocket(
	int af,
	int type,
	int protocol
) {
	// 调用原始函数
	SOCKET result = OldSocket(af, type, protocol);

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "Socket");
	pData->argNum = 3;

	strcpy(pData->argName[0], "Address Family");
	strcpy(pData->argName[1], "Socket Type");
	strcpy(pData->argName[2], "Protocol");

	sprintf(pData->argData[0], "Value: 0x%x", af);
	sprintf(pData->argData[1], "Value: 0x%x", type);
	sprintf(pData->argData[2], "Value: 0x%x", protocol);

	update();

	/*
	printf("\n\n****************************************\n");
	printf("socket Hooked\n");
	printf("Socket Parameters:\n");
	printf("Address Family: %d\n", af);
	printf("Socket Type: %d\n", type);
	printf("Protocol: %d\n", protocol);
	printf("****************************************\n\n");
	*/

	mtx.unlock();
	return result;
}

extern "C" __declspec(dllexport) int WINAPI NewBind(
	SOCKET s,
	const struct sockaddr* name,
	int namelen
) {
	// 调用原始函数
	int result = OldBind(s, name, namelen);

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "Bind");
	pData->argNum = 5;

	strcpy(pData->argName[0], "Socket Descriptor");
	strcpy(pData->argName[1], "name");
	strcpy(pData->argName[2], "namelen");
	strcpy(pData->argName[3], "IP Address");
	strcpy(pData->argName[4], "Port");

	sprintf(pData->argData[0], "Addr: 0x%llx", s);
	sprintf(pData->argData[1], "Addr: 0x%p", name);
	sprintf(pData->argData[2], "Value: 0x%x", namelen);

	/*
	printf("\n\n****************************************\n");
	printf("bind Hooked\n");
	printf("Socket Descriptor: %d\n", s);
	*/

	// 解析IP和端口
	if (name->sa_family == AF_INET) {
		struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
		char ip[INET_ADDRSTRLEN];
		const char* ip_str = inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
		if (ip_str != NULL) {
			sprintf(pData->argData[3], "%s", ip_str);
			// printf("IP Address: %s\n", ip_str);
		}
		sprintf(pData->argData[4], "%d", ntohs(addr_in->sin_port));
		// printf("Port: %d\n", ntohs(addr_in->sin_port));
	}
	else if (name->sa_family == AF_INET6) {
		struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
		char ip[INET6_ADDRSTRLEN];
		const char* ip_str = inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
		if (ip_str != NULL) {
			sprintf(pData->argData[3], "%s", ip_str);
			// printf("IP Address: %s\n", ip_str);
		}
		sprintf(pData->argData[4], "%d", ntohs(addr_in6->sin6_port));
		// printf("Port: %d\n", ntohs(addr_in6->sin6_port));
	}

	update();

	// printf("****************************************\n\n");

	mtx.unlock();
	return result;
}

extern "C" __declspec(dllexport) int WINAPI NewSend(
	SOCKET s,
	const char* buf,
	int len,
	int flags
) {
	// 调用原始函数
	int result = OldSend(s, buf, len, flags);

	// 将发送的内容转换为字符串
	std::string content(buf, len);
	// 检查发送的内容是否是已读取的文件内容
	if (fileContents.find(content) != fileContents.end())
	{
		// 在这里可以添加逻辑来处理将文件内容发送到网络的情况
		printf("Detected sending file content to network:\n%s\n", content.c_str());
	}

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "Send");
	pData->argNum = 4;

	strcpy(pData->argName[0], "Socket Descriptor");
	strcpy(pData->argName[1], "Data Length");
	strcpy(pData->argName[2], "Data");
	strcpy(pData->argName[3], "flags");

	sprintf(pData->argData[0], "Addr: 0x%llx", s);
	sprintf(pData->argData[1], "Value: 0x%x", len);
	sprintf(pData->argData[2], "Value: %s", buf);
	sprintf(pData->argData[3], "Value: 0x%x", flags);

	/*
	printf("\n\n****************************************\n");
	printf("send Hooked\n");
	printf("Socket Descriptor: %d\n", s);
	printf("Data Length: %d\n", len);
	printf("Data: %s\n", buf);
	printf("****************************************\n\n");
	*/

	update();
	mtx.unlock();
	return result;
}

extern "C" __declspec(dllexport) int WINAPI NewConnect(
	SOCKET s,
	const struct sockaddr* name,
	int namelen
) {
	// 调用原始函数
	int result = OldConnect(s, name, namelen);

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "Connect");
	pData->argNum = 5;

	strcpy(pData->argName[0], "Socket Descriptor");
	strcpy(pData->argName[1], "name");
	strcpy(pData->argName[2], "namelen");
	strcpy(pData->argName[3], "Target IP Address");
	strcpy(pData->argName[4], "Target Port");

	sprintf(pData->argData[0], "Addr: 0x%llx", s);
	sprintf(pData->argData[1], "Addr: 0x%p", name);
	sprintf(pData->argData[2], "Value: 0x%x", namelen);

	/*
	printf("\n\n****************************************\n");
	printf("connect Hooked\n");
	printf("Socket Descriptor: %d\n", s);
	*/

	// 解析目标IP和端口
	if (name->sa_family == AF_INET) {
		struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
		char ip[INET_ADDRSTRLEN];
		const char* ip_str = inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
		if (ip_str != NULL) {
			sprintf(pData->argData[3], "%s", ip_str);
			// printf("Target IP Address: %s\n", ip_str);
		}
		sprintf(pData->argData[4], "%d", ntohs(addr_in->sin_port));
		// printf("Target Port: %d\n", ntohs(addr_in->sin_port));
	}
	else if (name->sa_family == AF_INET6) {
		struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
		char ip[INET6_ADDRSTRLEN];
		const char* ip_str = inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
		if (ip_str != NULL) {
			sprintf(pData->argData[3], "%s", ip_str);
			// printf("Target IP Address: %s\n", ip_str);
		}
		sprintf(pData->argData[4], "%d", ntohs(addr_in6->sin6_port));
		// printf("Target Port: %d\n", ntohs(addr_in6->sin6_port));
	}

	// printf("****************************************\n\n");

	update();
	mtx.unlock();
	return result;
}

extern "C" __declspec(dllexport) int WINAPI NewRecv(
	SOCKET s,
	char* buf,
	int len,
	int flags
) {
	// 调用原始函数
	int result = OldRecv(s, buf, len, flags);

	mtx.lock();
	GetLocalTime(&(pData->st));
	pData->type = LOG_CUSTOM;

	strcpy(pData->funcName, "Recv");
	pData->argNum = 4;

	strcpy(pData->argName[0], "Socket Descriptor");
	strcpy(pData->argName[1], "Data Length");
	strcpy(pData->argName[2], "Data");
	strcpy(pData->argName[3], "flags");

	sprintf(pData->argData[0], "Addr: 0x%llx", s);
	sprintf(pData->argData[1], "Value: 0x%x", len);
	sprintf(pData->argData[2], "Value: %s", buf);
	sprintf(pData->argData[3], "Value: 0x%x", flags);

	/*
	printf("\n\n****************************************\n");
	printf("recv Hooked\n");
	printf("Socket Descriptor: %d\n", s);
	printf("Received Data Length: %d\n", result);
	printf("Received Data: %s\n", buf);
	printf("****************************************\n\n");
	*/

	update();
	mtx.unlock();
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
		sprintf(pData->message, "行为监测开始。");
		update();

		printf("行为监测开始。\n");

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

		// 网络操作
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

		// 网络操作
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
