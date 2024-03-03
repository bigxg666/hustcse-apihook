/*
* dllmain.cpp : 定义 DLL 应用程序的入口点。
*/

#include"pch.h"
#include"framework.h"
#include"detours.h"
#include"stdio.h"
#include"windows.h"
#include <string>
#include <set>
#include <iostream>

#pragma comment(lib,"detours.lib")

SYSTEMTIME st;

struct FileData {
	std::string filename;//完整文件名
	std::string folderPath;//文件夹路径
	std::string fileExtension;//文件扩展名
};
std::set<std::string> accessedFolders; // 存储已经访问过的文件夹路径

// 获取文件夹路径及文件后缀等信息
FileData GetFileData(HANDLE hFile) {
	FileData result;

	//会引起CreateFile操作，但在NewCreateFile函数中进行了过滤
	TCHAR szFileName[MAX_PATH];
	if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0) {
		std::wstring wsFileName(szFileName);
		std::string fileName(wsFileName.begin(), wsFileName.end());
		result.filename = fileName;
		// 去除路径前缀 \\?\ 
		if (result.filename.substr(0, 4) == "\\\\?\\")
			result.filename = result.filename.substr(4);

		// 从文件名中提取文件扩展名
		size_t dotPos = result.filename.find_last_of('.');
		if (dotPos != std::string::npos)
			result.fileExtension = result.filename.substr(dotPos);
		else
			std::cout << "Unable to extract the file extension." << std::endl;

		//获取文件夹路径
		size_t found = result.filename.find_last_of("\\/");
		if (found != std::string::npos) {
			result.folderPath = result.filename.substr(0, found);
			// 将文件夹路径添加到已访问文件夹集合中
			accessedFolders.insert(result.folderPath);
			//打印访问过的文件夹
			//std::cout << "Folder path: " << result.folderPath << std::endl;
		}
		else
			std::cout << "Unable to extract the folder path." << std::endl;
	}
	else
		std::cout << "Unable to get the file name." << std::endl;
	return result;
}

/*
* 实现文件操作监控
* OpenFile CreateFile ReadFile WriteFile DeleteFile
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
	if (wcsstr(lpFileName, L"log") != nullptr ||
		wcsstr(lpFileName, L".config") != nullptr ||
		wcsstr(lpFileName, L"MountPointManage") != nullptr) {
		// 如果包含需要过滤的字符串，则直接返回原函数
		return OldCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

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
	delete[] pchar;								// 注意释放内存，否则造成内存泄漏！
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
		FileData myFile = GetFileData(hObject);//要在关闭句柄之前进行
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
	/*
	* 增加条件判断，使得仅监测磁盘文件的读取
	* 否则将会出现大量无用信息
	*/
	if (GetFileType(hFile) == FILE_TYPE_DISK) {
		FileData myFile = GetFileData(hFile);

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

		std::cout << "Filename: " << myFile.filename << std::endl;//改为了输出文件名
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to read: " << nNumberOfBytesToRead << std::endl;
		std::cout << "Number of bytes read (out parameter): 0x" << lpNumberOfBytesRead << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

		//判断操作范围是否有多个文件夹
		if (accessedFolders.size() > 1) {
			//将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Operation spans multiple folders." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

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
		// 不建议使用该段代码，该段代码会引起CreateFile操作    
		/*
		TCHAR szFileName[MAX_PATH];
		if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED) != 0) {
			std::cout << "File being written to: " << szFileName << std::endl;
		}
		*/
		FileData myFile = GetFileData(hFile);

		printf("\n\n******************************************\n");
		printf("WriteFile Hooked\n");
		GetLocalTime(&st);
		printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		std::cout << "Filename: " << myFile.filename << std::endl;//改为了输出文件名
		std::cout << "Buffer address: 0x" << lpBuffer << std::endl;
		std::cout << "Number of bytes to write: " << nNumberOfBytesToWrite << std::endl;
		std::cout << "Number of bytes written (out parameter): 0x" << lpNumberOfBytesWritten << std::endl;
		std::cout << "Overlapped structure address: 0x" << lpOverlapped << std::endl;

		//判断操作范围是否有多个文件夹
		if (accessedFolders.size() > 1) {
			//将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Operation spans multiple folders." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

		//通过后缀来判断是否修改了可执行文件包括exe，dll，ocx等
		if (myFile.fileExtension == ".exe" || myFile.fileExtension == ".dll" || myFile.fileExtension == ".ocx") {
			//将输出字体颜色设置为红色
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cout << "Warning: Possible self-replication." << std::endl;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

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

	int num = WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, NULL, 0, NULL, FALSE);
	char* pchar = new char[num];
	WideCharToMultiByte(CP_OEMCP, NULL, lpFileName, -1, pchar, num, NULL, FALSE);
	std::cout << "File being deleted: " << pchar << std::endl;

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

//复制文件，用于检测是否存在自我复制的情况
static BOOL(WINAPI* OldCopyFile)(
	LPCTSTR lpExistingFileName,          //现有文件名称
	LPCTSTR lpNewFileName,               //新文件名称
	BOOL beFailIfExist
	) = CopyFile;

extern "C" __declspec(dllexport) BOOL WINAPI NewCopyFile(
	LPCTSTR lpExistingFileName,          //现有文件名称
	LPCTSTR lpNewFileName,               //新文件名称
	BOOL beFailIfExist
) {
	BOOL result = OldCopyFile(lpExistingFileName, lpNewFileName, beFailIfExist);

	printf("\n\n******************************************\n");
	printf("CopyFile Hooked\n");
	GetLocalTime(&st);
	printf("DLL日志输出:%d-%d-%d %02d: %02d: %02d: %03d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	if (result) {
		//将输出字体颜色设置为红色
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		std::cout << "Warning: Self-copy detected." << std::endl;
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
	else {
		std::cout << "Failed to copy file." << std::endl;
	}

	printf("******************************************\n\n");
	return result;
}

// 主函数
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		// 文件操作
		DetourAttach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourAttach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourAttach(&(PVOID&)OldReadFile, NewReadFile);
		DetourAttach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourAttach(&(PVOID&)OldDeleteFile, NewDeleteFile);
		DetourAttach(&(PVOID&)OldCopyFile, NewCopyFile);

		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		// 文件操作
		DetourDetach(&(PVOID&)OldCreateFile, NewCreateFile);
		DetourDetach(&(PVOID&)OldCopyFile, NewCopyFile);
		DetourDetach(&(PVOID&)OldCloseHandle, NewCloseHandle);
		DetourDetach(&(PVOID&)OldReadFile, NewReadFile);
		DetourDetach(&(PVOID&)OldWriteFile, NewWriteFile);
		DetourDetach(&(PVOID&)OldDeleteFile, NewDeleteFile);
		DetourDetach(&(PVOID&)OldCopyFile, NewCopyFile);

		DetourTransactionCommit();
		break;
	}

	return TRUE;
}
