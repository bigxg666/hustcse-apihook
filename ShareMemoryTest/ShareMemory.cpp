#include <iostream>
#include <windows.h>

using namespace std;

#pragma pack(1)

const uint8_t LOG_CUSTOM = 0x00;
const uint8_t LOG_WARNING = 0x01;
const uint8_t LOG_ERROR = 0x02;

const int MAXLEN = 65536;

typedef struct SharedData{
	uint8_t type;
	char data[MAXLEN];
}SharedData;

#pragma pack()

int main() {
    // 创建共享内存对象
    HANDLE hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), "MySharedMemory");
    if (hSharedMemory == NULL) {
        cout << "Failed to create shared memory." << endl;
        return 1;
    }

    // 将共享内存映射到当前进程的地址空间
    LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);
    if (pSharedMemory == NULL) {
        cout << "Failed to map shared memory." << endl;
        CloseHandle(hSharedMemory);
        return 1;
    }

    // 将数据写入共享内存
    SharedData* pData = static_cast<SharedData*>(pSharedMemory);
    pData->type = LOG_WARNING;
	sprintf(pData->data, "****************************************\n");
	sprintf((pData->data) + strlen(pData->data), "RegCreateKeyExA Hooked\n");
	sprintf((pData->data) + strlen(pData->data), "此程序存在新增注册表项操作\n");
	sprintf((pData->data) + strlen(pData->data), "此程序存在修改注册表自启动项的操作\n");
	strncpy((pData->data) + strlen(pData->data), "****************************************", sizeof(pData->data));

	while(true);
	
    // 解除共享内存的映射
    UnmapViewOfFile(pSharedMemory);

    // 关闭共享内存对象
    CloseHandle(hSharedMemory);

    std::cout << "Data has been written to shared memory." << std::endl;
	
    return 0;
}
