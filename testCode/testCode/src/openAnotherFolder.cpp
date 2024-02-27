#include <head.h>
extern void openAnotherFolder() {
    HANDLE hOpenFile = (HANDLE)CreateFile(L".\\testFolder\\test.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
    CloseHandle(hOpenFile);
}