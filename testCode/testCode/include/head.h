#include <windows.h>
#include <iostream>
#include <stdlib.h>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#define PAGE_SIZE  4096
//正常操作
void showMenu();
void writeFileString();
void readFileString();
void heapCreateAndDestory();
void regCreateAndSetValue();
void regOpenAndDelValue();
void recvData();
void sendData();
//恶意操作
void modifyExProgram();
void selfReplication();
void openAnotherFolder();
void headRepeatedRelease();
void modifyStartupRegistry();
