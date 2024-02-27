#include <head.h>
extern void modifyStartupRegistry() {
    HKEY hKey = NULL;
    size_t lRet = RegOpenKeyEx(HKEY_CURRENT_USER, (LPWSTR)L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
    if (lRet == ERROR_SUCCESS) {
        printf("Open successfully!\n");
    }
    else {
        printf("Open failed\n");
    }
    RegCloseKey(hKey);
}