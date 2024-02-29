// ReadLog.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

const uint8_t LOG_INFO = 0x00;
const uint8_t LOG_CUSTOM = 0x01;
const uint8_t LOG_WARNING = 0x02;
const uint8_t LOG_ERROR = 0x03;

const uint8_t CTRL_QUIT = 0x10;

const int MAX_LEN = 65536;
const int MAX_ARG_NUM = 16;				// 最多支持16个参数
const int MAX_ARGNAME_LEN = 128;		// 每个参数的名称最大支持128字节
const int MAX_ARGDATA_LEN = 512;		// 每个参数的数据最大支持512字节

typedef struct SharedData {
	bool updateFlag = false;			// 更新标志
	BYTE type = 0;						// 日志类型

	char data[MAX_LEN] = {};			// 提示信息

	BYTE argNum = 0;					// 参数个数，仅在type为LOG_CUSTOM时有效
	BYTE argName[MAX_ARG_NUM][MAX_ARGNAME_LEN] = {};		// 参数名称表
	BYTE argData[MAX_ARG_NUM][MAX_ARGDATA_LEN] = {};		// 参数数据表
}SharedData;

SharedData recvData;

int main() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出句柄

	// 获取默认文本属性
	CONSOLE_SCREEN_BUFFER_INFO defaultInfo;
	GetConsoleScreenBufferInfo(hConsole, &defaultInfo);

	// 打开共享内存对象
	HANDLE hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
	while (hSharedMemory == NULL) {
		std::cout << "Failed to open shared memory." << std::endl;
		system("pause");
		hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
	}

	// 将共享内存映射到当前进程的地址空间
	LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0, 0, 0);
	if (pSharedMemory == NULL) {
		std::cout << "Failed to map shared memory." << std::endl;
		CloseHandle(hSharedMemory);
		return EXIT_FAILURE;
	}

	bool backFlag = false;

	// 读取共享内存中的数据
	while (true) {
		SharedData* pData = static_cast<SharedData*>(pSharedMemory);

		if (pData->type == CTRL_QUIT) break;

		if (pData->updateFlag == backFlag) continue;

		if (pData->type == LOG_CUSTOM) {
			// 白色字体显示
			// 恢复默认文本属性
			SetConsoleTextAttribute(hConsole, defaultInfo.wAttributes);
		}
		else if (pData->type == LOG_WARNING) {
			// 红色字体显示
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		}
		else if (pData->type == LOG_ERROR) {
			// ？色字体显示
			SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
		}
		std::cout << pData->data << std::endl;

		backFlag = pData->updateFlag;
	}

	// 解除共享内存的映射
	UnmapViewOfFile(pSharedMemory);

	// 关闭共享内存对象
	CloseHandle(hSharedMemory);

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
