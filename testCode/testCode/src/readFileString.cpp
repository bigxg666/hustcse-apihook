#include <head.h>
extern void readFileString(){
    CHAR* pBuffer;
    int fileSize = 0;
    bool flag;
    HANDLE hOpenFile = (HANDLE)CreateFile(L"test.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    if (hOpenFile == INVALID_HANDLE_VALUE)
    {
        hOpenFile = NULL;
        printf("Can not open the file\n");
        return;
    }
    printf("Successfully open a file\n");
    fileSize = GetFileSize(hOpenFile, NULL);
    pBuffer = (char*)malloc((fileSize + 1) * sizeof(char));
    flag = ReadFile(hOpenFile, pBuffer, fileSize, NULL, NULL);
    pBuffer[fileSize] = 0;
    if (flag) {
        printf("Successfully read a string:%s!\n", pBuffer);
    }
    free(pBuffer);
    CloseHandle(hOpenFile);
}
