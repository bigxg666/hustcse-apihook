#include <iostream>
#include <windows.h>

using namespace std;

const uint8_t LOG_CUSTOM = 0x00;
const uint8_t LOG_WARNING = 0x01;
const uint8_t LOG_ERROR = 0x02;

const int MAXLEN = 65536;

typedef struct SharedData{
	uint8_t type;
	char data[MAXLEN];
}SharedData;

int main() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出句柄
	
	// 获取默认文本属性
	CONSOLE_SCREEN_BUFFER_INFO defaultInfo;
	GetConsoleScreenBufferInfo(hConsole, &defaultInfo);
	
	// 打开共享内存对象
	HANDLE hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, "MySharedMemory");
	if (hSharedMemory == NULL) {
		cout << "Failed to open shared memory." << endl;
		return EXIT_FAILURE;
	}
	
	// 将共享内存映射到当前进程的地址空间
	LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0,  0, 0);
	if (pSharedMemory == NULL) {
		cout << "Failed to map shared memory." << endl;
		CloseHandle(hSharedMemory);
		return EXIT_FAILURE;
	}
	
	// 读取共享内存中的数据
	SharedData* pData = static_cast<SharedData*>(pSharedMemory);
	if(pData->type == LOG_CUSTOM){
		// 白色字体显示
		cout << "[日志] " << endl;
	}else if(pData->type == LOG_WARNING){
		// 红色字体显示
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		cout << "[警告] " << endl;
	}else if(pData->type == LOG_ERROR){
		// ？色字体显示
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
		cout << "[错误] " << endl;
	}
	cout << pData->data << endl;
	
	// 恢复默认文本属性
	SetConsoleTextAttribute(hConsole, defaultInfo.wAttributes);
	
	// 解除共享内存的映射
	UnmapViewOfFile(pSharedMemory);
	
	// 关闭共享内存对象
	CloseHandle(hSharedMemory);
	
	return 0;
}

