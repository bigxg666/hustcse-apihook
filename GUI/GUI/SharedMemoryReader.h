#ifndef SHAREDMEMORYREADER_H
#define SHAREDMEMORYREADER_H

#define OUTPUT_TO_FILE

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <Windows.h>

const BYTE LOG_UNINIT = 0xFF;

const BYTE LOG_INFO = 0x00;
const BYTE LOG_CUSTOM = 0x01;
const BYTE LOG_WARNING = 0x02;
const BYTE LOG_ERROR = 0x03;

const int MAX_CNT_NUM = UINT8_MAX;

const BYTE CTRL_QUIT = 0x10;

const int MAX_MSG_NUM = 16;				// 同一时刻最多提供16条提示信息
const int MAX_MSG_LEN = 512;			// 消息的最大长度

const int MAX_FUNCNAME_LEN = 128;		// 函数名最大支持128字节
const int MAX_ARG_NUM = 16;				// 最多支持16个参数
const int MAX_ARGNAME_LEN = 128;		// 每个参数的名称最大支持128字节
const int MAX_ARGDATA_LEN = 512;		// 每个参数的数据最大支持512字节

// uint8_t cnt = 0;
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

class SharedMemoryReader : public QObject
{
    Q_OBJECT

public:
    explicit SharedMemoryReader(QObject *parent = nullptr);

public slots:
    void readSharedMemory();
    // void update();

signals:
    void messageReceived(QString message);
    void API_Received(LogInfo *API_Info);

private:
    QString logFilePath = "./log.txt";
};

#endif // SHAREDMEMORYREADER_H
