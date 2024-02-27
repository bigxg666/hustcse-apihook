/*
 * @Author: kingqaquuu
 * @Date: 2024-02-27 13:19:02
 * @FilePath: \hustcse-apihook\testCode\testCode\src\writeFileString.cpp
 */
#include <head.h>
using namespace std;
extern void  writeFileString(){
    CHAR* pBuffer;
    int fileSize = 0;
    char writeString[100];
    bool flag;
    HANDLE hOpenFile = (HANDLE)CreateFile(L"test.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    if (hOpenFile == INVALID_HANDLE_VALUE)
    {
        hOpenFile = NULL;
        printf("Can not open the file\n");
        return;
    }
    printf("Successfully open a file\n");
    printf("Input a string:");
    scanf("%s", writeString);
    flag = WriteFile(hOpenFile, writeString, strlen(writeString), NULL, NULL);
    if (flag) {
        printf("Successfully writed!\n");
    }
    FlushFileBuffers(hOpenFile);
    CloseHandle(hOpenFile);
}
