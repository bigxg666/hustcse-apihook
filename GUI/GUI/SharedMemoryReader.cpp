#include "SharedMemoryReader.h"


// 在这里，你需要将原来的 SharedData 结构体替换为新的结构体
// 你还需要在读取共享内存中的数据时，根据新的结构体来解析数据

SharedMemoryReader::SharedMemoryReader(QObject *parent)
    : QObject(parent)
{
}

void SharedMemoryReader::readSharedMemory()
{
    qDebug() << "thread is running";
    HANDLE hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
    while (hSharedMemory == NULL) {
        qDebug() << "未检测到共享内存！";
        Sleep(1000);
        hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, L"MySharedMemory");
    }
    // qDebug() << "success";
    LPVOID pSharedMemory = MapViewOfFile(hSharedMemory, FILE_MAP_READ, 0, 0, 0);
    if (pSharedMemory == NULL) {
        qDebug() << "映射共享内存失败！";
        CloseHandle(hSharedMemory);
        return;
    }
    // qDebug() << "success";
    SharedData* pSharedData = static_cast<SharedData*>(pSharedMemory);

    uint32_t cnt = 0;
    LogInfo* pData = &(pSharedData->logInfoList[0]);

    while (true) {
        if (pSharedData->cnt == cnt) continue;

        while (cnt != pSharedData->cnt) {

            // qDebug() << "success";
            QString logMessage;
            QTextStream logStream(&logMessage);

            logStream << "[" << pData->st.wYear << "-" << pData->st.wMonth << "-" << pData->st.wDay << " "
                      << pData->st.wHour << ":" << pData->st.wMinute << ":" << pData->st.wSecond <<  "] ";
            // qDebug() << pData->st.wMonth;
            // Sleep(1000);
            QTextCodec *codec = QTextCodec::codecForName("GBK");
            if (pData->type == LOG_CUSTOM) {
                logStream << "[截获] 该程序使用了函数: " << pData->funcName << "\n";
                // qDebug() << LOG_CUSTOM;
                emit API_Received(pData);
                for (int j = 0; j < pData->argNum; j++) {
                    QString argName = codec->toUnicode(pData->argName[j]);
                    QString argData = codec->toUnicode(pData->argData[j]);
                    logStream << QString("%1 %2: %3\n").arg("", 32).arg(argName).arg(argData);
                }
            }
            else if (pData->type == LOG_INFO){
                QString message = codec->toUnicode(pData->message);

                logStream << "[信息] " << message << "\n";
            }
            else if(pData->type == LOG_ERROR){
                QString message = codec->toUnicode(pData->message);

                logStream << "[错误]" << message << "\n";
                emit messageReceived(logMessage);
            }
            else if(pData->type == LOG_WARNING){
                QString message = codec->toUnicode(pData->message);

                logStream << "[警告]" << message << "\n";
                emit messageReceived(logMessage);
            }
            else{
                continue;
            }

#ifdef OUTPUT_TO_FILE
            QFile logFile(logFilePath);

            if (!logFile.exists()) {
                logFile.open(QIODevice::WriteOnly);
                logFile.close();
            }

            if (logFile.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream fileStream(&logFile);
                fileStream << logMessage;
                logFile.close();
            }
#else
            qDebug() << logMessage.trimmed();
#endif
            cnt++;
            cnt %= MAX_CNT_NUM;
            pData = &(pSharedData->logInfoList[cnt]);
        }

        Sleep(1000);
    }

    UnmapViewOfFile(pSharedMemory);

    CloseHandle(hSharedMemory);
}

