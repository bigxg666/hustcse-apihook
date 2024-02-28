//dllmain.cpp:定义DLL应用程序的入口点
#include"pch.h"
#include"framework.h"
#include"detours.h"
#include"stdio.h"
#include"windows.h"
#include<iostream>

#pragma comment(lib,"detours.lib")

SYSTEMTIME st;

//文件操作 OpenFile CreateFile
//创建文件
static HANDLE(WINAPI* OldCreateFile)(
	LPCTSTR lpFileName,							//文件名
	DWORD dwDesiredAccess,					    //访问模式
	DWORD dwShareMode,							//共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	//安全属性（即销毁方式）
	DWORD dwCreationDisposition,				//how to create
	DWORD dwFlagsAndAttributes,					//文件属性
	HANDLE hTemplateFile						//模板文件句柄
	) = CreateFile;

extern "C" __declspec(dllexport) HANDLE WINAPI NewCreateFile(
	LPCTSTR lpFileName,							//文件名
	DWORD dwDesiredAccess,					    //访问模式
	DWORD dwShareMode,							//共享模式
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	//安全属性（即销毁方式）
	DWORD dwCreationDisposition,				//how to create
	DWORD dwFlagsAndAttributes,					//文件属性
	HANDLE hTemplateFile						//模板文件句柄
)
{
	printf("\n\n******************************************\n");
	printf("CreateFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);


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

//关闭文件
static BOOL(WINAPI* OldCloseHandle)(HANDLE hObject) = CloseHandle;

extern "C" __declspec(dllexport) BOOL WINAPI NewCloseHandle(HANDLE hObject)
{

	printf("\n\n******************************************\n");
	printf("CloseHandle Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "Handle being closed: 0x" << hObject << std::endl;

	BOOL result = OldCloseHandle(hObject);
	if (result)
	{
		std::cout << "Handle closed successfully" << std::endl;
	}
	else
	{
		std::cout << "Failed to close handle. Error code: " << GetLastError() << std::endl;
	}

	printf("******************************************\n\n");
	return result;
}

//读文件
static BOOL(WINAPI* OldReadFile)(
	HANDLE       hFile,                         //文件句柄 
	LPVOID       lpBuffer,                      //读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          //要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
	) = ReadFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewReadFile(
	HANDLE       hFile,                         //文件句柄 
	LPVOID       lpBuffer,                      //读取文件数据存储的内存缓冲区
	DWORD        nNumberOfBytesToRead,          //要读取的最多字节数
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
)
{
	printf("\n\n******************************************\n");
	printf("ReadFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	TCHAR szFileName[MAX_PATH];
	if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0)
	{
		std::cout << "File being read: " << szFileName << std::endl;
	}

	std::cout << "Number of bytes to read: " << nNumberOfBytesToRead << std::endl;
	std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
	std::cout << "Number of bytes read (out parameter): 0x" << lpNumberOfBytesRead << std::endl;
	std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

	printf("******************************************\n\n");
	return OldReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

//写文件
static BOOL(WINAPI* OldWriteFile)(
	HANDLE       hFile,                         //文件句柄 
	LPCVOID       lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
	) = WriteFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewWriteFile(
	HANDLE       hFile,                         //文件句柄 
	LPCVOID       lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
)
{
	printf("\n\n******************************************\n");
	printf("ReadFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	TCHAR szFileName[MAX_PATH];
	if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0)
	{
		std::cout << "File being written to: " << szFileName << std::endl;
	}

	std::cout << "Number of bytes to write: " << nNumberOfBytesToWrite << std::endl;
	std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
	std::cout << "Number of bytes written (out parameter): 0x" << lpNumberOfBytesWritten << std::endl;
	std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

	printf("******************************************\n\n");
	return OldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

//删除文件
static BOOL(WINAPI* OldDeleteFile)(LPCTSTR lpFileName) = DeleteFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewDeleteFile(LPCTSTR lpFileName)
{
	printf("\n\n******************************************\n");
	printf("DeleteFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	std::cout << "File being deleted: " << lpFileName << std::endl;

	BOOL result = OldDeleteFile(lpFileName);
	if (result)
	{
		std::cout << "File deleted successfully" << std::endl;
	}
	else
	{
		std::cout << "Failed to delete file. Error code: " << GetLastError() << std::endl;
	}

	printf("******************************************\n\n");
	return result;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourAttach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourAttach(&(PVOID&)OldReadFile, NewReadFile);
		DetourAttach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourAttach(&(PVOID&)OldDeleteFile, NewDeleteFile);
		DetourTransactionCommit();
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH: {

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourDetach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourDetach(&(PVOID&)OldReadFile, NewReadFile);
		DetourDetach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourDetach(&(PVOID&)OldDeleteFile, NewDeleteFile);
		DetourTransactionCommit();
		break;
	}
	}
	return TRUE;
}