#include <iostream>
#include <windows.h>

using namespace std;

const uint8_t LOG_CUSTOM = 0x00;
const uint8_t LOG_WARNING = 0x01;
const uint8_t LOG_ERROR = 0x02;

const uint8_t CTRL_QUIT = 0x10;

const int MAX_LEN = 65536;
const int MAX_ARG_NUM = 16;				// 最多支持16个参数
const int MAX_ARGNAME_LEN = 128;		// 每个参数的名称最大支持128字节
const int MAX_ARGDATA_LEN = 512;		// 每个参数的数据最大支持512字节

typedef struct SharedData {
	bool updateFlag = false;			// 更新标志
	BYTE type = 0;						// 日志类型
	
	char data[MAX_LEN];					// 提示信息
	
	BYTE argNum = 0;					// 参数个数，仅在type为LOG_CUSTOM时有效
	BYTE argName[MAX_ARG_NUM][MAX_ARGNAME_LEN];		// 参数名称表
	BYTE argData[MAX_ARG_NUM][MAX_ARGDATA_LEN];		// 参数数据表
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
	LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0, 0, 0);
	if (pSharedMemory == NULL) {
		cout << "Failed to map shared memory." << endl;
		CloseHandle(hSharedMemory);
		return EXIT_FAILURE;
	}
	
	bool backFlag = false;
	
	// 读取共享内存中的数据
	while(true){
		SharedData* pData = static_cast<SharedData*>(pSharedMemory);
		
		if(pData->type == CTRL_QUIT) break;
		
		if(pData->updateFlag == backFlag) continue;
		
		if(pData->type == LOG_CUSTOM){
			// 白色字体显示
			// 恢复默认文本属性
			SetConsoleTextAttribute(hConsole, defaultInfo.wAttributes);
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
		
		backFlag = pData->updateFlag;
	}
	
	// 解除共享内存的映射
	UnmapViewOfFile(pSharedMemory);
	
	// 关闭共享内存对象
	CloseHandle(hSharedMemory);
	
	return 0;
}

