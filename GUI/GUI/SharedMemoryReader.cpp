#include "SharedMemoryReader.h"


// 在这里，你需要将原来的 SharedData 结构体替换为新的结构体
// 你还需要在读取共享内存中的数据时，根据新的结构体来解析数据

SharedMemoryReader::SharedMemoryReader(QObject *parent)
    : QObject(parent)
{
}



void SharedMemoryReader::readSharedMemory()
{
    // 获取标准输出句柄
    // HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    qDebug() << "thread is running";
    // 打开共享内存对象
    HANDLE hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
    while (hSharedMemory == NULL) {
        qDebug() << "未检测到共享内存！";
        Sleep(1000);
        hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
    }

    // 将共享内存映射到当前进程的地址空间
    LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0, 0, 0);
    if (pSharedMemory == NULL) {
        qDebug() << "映射共享内存失败！";
        CloseHandle(hSharedMemory);
        return;
    }

    bool backFlag = false;
    uint8_t cnt = 0;

    // 读取共享内存中的数据
    while (true) {
        SharedData* pData = static_cast<SharedData*>(pSharedMemory);
        if (pData->cnt == cnt) continue;
        // while (cnt != pData->cnt) {
        // 遍历日志表中的所有日志信息
        for (int i = cnt; i < pData->cnt; i++) {
            qDebug() << pData->cnt;
            LogInfo* pLogInfo = &(pData->logInfoList[i]);

            QString logMessage;
            QTextStream logStream(&logMessage);

            logStream << "[" << pLogInfo->st.wYear << "-" << pLogInfo->st.wMonth << "-" << pLogInfo->st.wDay << " "
                      << pLogInfo->st.wHour << ":" << pLogInfo->st.wMinute << ":" << pLogInfo->st.wSecond <<  "] ";

            if (pLogInfo->type == LOG_CUSTOM) {
                logStream << "[截获] 该程序使用了函数: " << pLogInfo->funcName << "\n";

                emit API_Received(pLogInfo);
                for (int j = 0; j < pLogInfo->argNum; j++) {
                    logStream << QString("%1参数名: %2, 内容- %3\n").arg("", 32).arg(pLogInfo->argName[j]).arg(pLogInfo->argData[j]);
                }
            } else if (pLogInfo->type == LOG_INFO){
                logStream << "[信息] " << pLogInfo->message << "\n";
            }
            else if(pLogInfo->type == LOG_ERROR){
                logStream << "[错误]" << pLogInfo->message << "\n";
                emit messageReceived(logMessage);
            }
            else if(pLogInfo->type == LOG_WARNING){
                logStream << "[警告]" << pLogInfo->message << "\n";
            }
            else{
                continue;
            }

#ifdef OUTPUT_TO_FILE
            QFile logFile("C:/Users/zhuor/Desktop/secure_design/Test_GUI/GUI/log.txt");
            if (logFile.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream fileStream(&logFile);
                fileStream << logMessage;
                logFile.close();
            }
#else
            qDebug() << logMessage.trimmed();
#endif
        }
        cnt = pData->cnt;

        // 等待新的数据

        Sleep(1000);
    }

    // 解除共享内存的映射
    UnmapViewOfFile(pSharedMemory);

    // 关闭共享内存对象
    CloseHandle(hSharedMemory);
}
