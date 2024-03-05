#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <windows.h>
#include <stdlib.h>
#include <WS2tcpip.h>
#include<winsock.h>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    MessageBoxA(NULL, "I'm MessageBoxA", "MessageBoxA", MB_OK);
}

void CallMsgBox(LPCWSTR message)
{
    MessageBoxW(NULL, message, L"debug", MB_OK);
}

void MainWindow::on_pushButton_2_clicked()
{
    CallMsgBox(L"这是MessageBoxW");
}



void MainWindow::on_pushButton_3_clicked()
{
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, 100, 100);
    CallMsgBox(L"HeapCreate执行");
    if (hHeap != NULL){
        auto hptest = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 100);
        CallMsgBox(L"HeapAlloc执行");
        HeapFree(hHeap, 0, hptest);
        CallMsgBox(L"HeapFree执行");
        HeapDestroy(hHeap);
        CallMsgBox(L"HeapDestroy执行");
    }else{
        CallMsgBox(L"创建堆失败");
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    HANDLE hFile = CreateFileA(
        "test.txt",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    CallMsgBox(L"已创建test.txt");
    DWORD sz = 0;
    char buf1[4096];
    memcpy(buf1, "Hello World",12);
    WriteFile(hFile, buf1, 12, &sz, NULL);
    FlushFileBuffers(hFile);
    CallMsgBox(L"已将Hello World写入文件test.txt");
    LONG lDistance = 0;
    SetFilePointer(hFile, lDistance, NULL, FILE_BEGIN);
    char buf[20];
    DWORD dw = 0;
    ReadFile(hFile, buf, 12, &dw, NULL);
    CallMsgBox(L"读文件已执行");
    CloseHandle(hFile);
    CallMsgBox(L"文件已关闭");
    DeleteFile(TEXT("test.txt"));
    CallMsgBox(L"test.txt已删除");
}


void MainWindow::on_pushButton_5_clicked()
{
    HKEY hk = NULL;
    HKEY hk2 = NULL;
    LPCSTR keyname = "SOFTWARE\\HUST\\Information_2104";
    unsigned long dwType = REG_SZ;
    char buffer[257] = { 0 };
    DWORD dwLen = 256;

    RegCreateKeyExA(HKEY_LOCAL_MACHINE, keyname , 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, NULL);
    CallMsgBox(L"已创建注册表项");

    RegCloseKey(hk);
    CallMsgBox(L"已关闭注册表项");

    RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, KEY_ALL_ACCESS, &hk2);
    CallMsgBox(L"已重新打开注册表项");

    RegSetValueExA(hk2, "Hello", 0, REG_SZ, (PBYTE)"Hello_HUST", 11);
    CallMsgBox(L"注册表项已改为Hello");

    RegQueryValueExA(hk2, "Hello", NULL, &dwType, (BYTE*)buffer, &dwLen);
    CallMsgBox(L"查询注册表项");

    RegCloseKey(hk2);
    CallMsgBox(L"注册表已关闭");
}

// 堆异常操作
void MainWindow::on_pushButton_6_clicked()
{
    const int PAGE_SIZE = 4096;
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 5, PAGE_SIZE * 50);
    CallMsgBox(L"尝试创建堆...");
    if (hHeap == NULL) {
        CallMsgBox(L"堆创建失败！");
        return;
    }
    CallMsgBox(L"成功创建堆！");

    CallMsgBox(L"尝试分配堆内存...");
    int* ptr = (int*)HeapAlloc(hHeap, 0, sizeof(int) * 20);
    if (ptr == NULL) {
        CallMsgBox(L"堆内存分配失败！");
        return;
    }
    CallMsgBox(L"成功分配堆内存！");

    for (int i = 0; i < 20; ++i) {
        ptr[i] = i;
    }

    if (!HeapFree(hHeap, 0, ptr)) {
        CallMsgBox(L"第一次释放堆内存失败！");
        return;
    }
    CallMsgBox(L"第一次释放堆内存！");

    if (!HeapFree(hHeap, 0, ptr)) {
        CallMsgBox(L"第二次释放堆内存失败！");
        return;
    }
    CallMsgBox(L"第二次释放堆内存！");
    HeapDestroy(hHeap);
    CallMsgBox(L"堆已被销毁！");
}


void MainWindow::on_pushButton_7_clicked()
{
    HANDLE hOpenFile = (HANDLE)CreateFile(L"a.exe", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    char buffer[100] = "Hello, World!";
    DWORD bytesWritten=0;
    WriteFile(hOpenFile,buffer,sizeof(buffer),&bytesWritten,NULL);
    CallMsgBox(L"已将Hello World写入a.exe");
    CloseHandle(hOpenFile);
}




void MainWindow::on_pushButton_8_clicked()
{
    HANDLE hFile = CreateFileA(
        "test.txt",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    CloseHandle(hFile);
    if(CopyFile(L"test.txt",L"a.txt",FALSE)){
    CallMsgBox(L"已复制test.txt");
    }
}


void MainWindow::on_pushButton_9_clicked()
{
    //定义长度变量
    int send_len = 0;
    int recv_len = 0;
    //定义发送缓冲区和接受缓冲区
    char send_buf[100] = "Hello World";
    char recv_buf[100];
    //定义服务端套接字，接受请求套接字
    SOCKET s_server;
    //服务端地址客户端地址
    SOCKADDR_IN server_addr;

    WORD w_req = MAKEWORD(2, 2);//版本号
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0) {
        return;
    }
    else {
        CallMsgBox(L"初始化套接字库成功！");
    }
    //检测版本号
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
        CallMsgBox(L"套接字库版本号不符！");
        WSACleanup();
        return;
    }
    else {
        CallMsgBox(L"套接字库版本正确！");
    }

    //填充服务端信息
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1", &server_addr.sin_addr.S_un.S_addr);
    server_addr.sin_port = htons(1234);
    //创建套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);
    //发送,接收数据
    send_len = send(s_server, send_buf, strlen(send_buf)+1, 0);
    if (send_len < 0) {
        CallMsgBox(L"发送失败！");
    }
    recv_len = recv(s_server, recv_buf, 100, 0);
    if (recv_len < 0) {
        CallMsgBox(L"接受失败！");
    }
    else {
        CallMsgBox(L"服务端信息已接收");
    }
}


void MainWindow::on_pushButton_10_clicked()
{
    HKEY hk = NULL;
    HKEY hk2 = NULL;
    LPCSTR keyname = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    unsigned long dwType = REG_SZ;
    char buffer[257] = { 0 };
    DWORD dwLen = 256;

    // 启动项
    RegCreateKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, NULL);
    CallMsgBox(L"创建启动项注册表");

    RegCloseKey(hk);
    CallMsgBox(L"关闭启动项注册表");

    RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyname, 0, KEY_ALL_ACCESS, &hk2);
    CallMsgBox(L"重新打开启动项注册表");

    RegSetValueExA(hk2, "Hello", 0, REG_SZ, (PBYTE)"Hello HUST", 11);
    CallMsgBox(L"修改启动项注册表");

    RegCloseKey(hk2);
    CallMsgBox(L"再次关闭启动项注册表");
}


void MainWindow::on_pushButton_11_clicked()
{
    HANDLE hOpenFile = (HANDLE)CreateFile(L".\\testFolder\\test.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    char buf[20];
    DWORD dw = 0;
    ReadFile(hOpenFile,buf,sizeof(buf),&dw,NULL);
    CallMsgBox(L"已读testFolder下test.txt文件");
    CloseHandle(hOpenFile);
}

