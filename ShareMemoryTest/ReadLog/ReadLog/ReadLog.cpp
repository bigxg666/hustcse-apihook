// ReadLog.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

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

int main() {
	// 获取标准输出句柄
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// 打开共享内存对象
	HANDLE hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
	while (hSharedMemory == NULL) {
		printf("未检测到共享内存！\n");
		system("pause");
		hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
	}

	// 将共享内存映射到当前进程的地址空间
	LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0, 0, 0);
	if (pSharedMemory == NULL) {
		printf("映射共享内存失败！\n");
		CloseHandle(hSharedMemory);
		return EXIT_FAILURE;
	}

	bool backFlag = false;

	// 读取共享内存中的数据
	while (true) {
		SharedData* pData = static_cast<SharedData*>(pSharedMemory);

		if (pData->updateFlag == backFlag) continue;

		if (pData->type == LOG_CUSTOM) {
			// 设置文本颜色为黄色
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
			printf("[%d-%d-%d %02d: %02d: %02d: %03d] ", pData->st.wYear, pData->st.wMonth, pData->st.wDay, pData->st.wHour, pData->st.wMinute, pData->st.wSecond, pData->st.wMilliseconds);
			printf("[截获] 该程序使用了函数: %s \n", pData->funcName);
			for (int i = 0; i < pData->argNum; i++) {
				std::cout << "\t参数名: " << pData->argName[i] << ", 内容-" << pData->argData[i] << std::endl;
			}
		}
		if (pData->type == LOG_WARNING) {
			// 设置文本颜色为红色
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			for (int i = 0; i < pData->msgNum; i++) {
				printf("[%d-%d-%d %02d: %02d: %02d: %03d] ", pData->st.wYear, pData->st.wMonth, pData->st.wDay, pData->st.wHour, pData->st.wMinute, pData->st.wSecond, pData->st.wMilliseconds);
				printf("[警告] %s\n", pData->message[i]);
			}
			
		}
		if (pData->type == LOG_INFO) {
			// 设置文本颜色为白色
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			for (int i = 0; i < pData->msgNum; i++) {
				printf("[%d-%d-%d %02d: %02d: %02d: %03d] ", pData->st.wYear, pData->st.wMonth, pData->st.wDay, pData->st.wHour, pData->st.wMinute, pData->st.wSecond, pData->st.wMilliseconds);
				printf("[信息] %s\n", pData->message[i]);
			}
		}

		backFlag = pData->updateFlag;
	}

	// 解除共享内存的映射
	UnmapViewOfFile(pSharedMemory);

	// 关闭共享内存对象
	CloseHandle(hSharedMemory);

	return 0;
}
