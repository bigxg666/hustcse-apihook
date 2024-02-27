#include <head.h>
extern void selfReplication() {
    HANDLE hOpenFile = (HANDLE)CreateFile(L"hello.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    CloseHandle(hOpenFile);
}