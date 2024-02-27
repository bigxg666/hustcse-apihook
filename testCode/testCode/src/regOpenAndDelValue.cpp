#include <head.h>
extern void regOpenAndDelValue() {
    HKEY hKey = NULL;
    size_t lRet = RegOpenKeyEx(HKEY_CURRENT_USER, (LPWSTR)L"Mykey", 0, KEY_ALL_ACCESS, &hKey);
    if (lRet == ERROR_SUCCESS) {
        printf("open successfully!\n");
    }
    else {
        printf("open failed\n");
    }
    lRet = RegDeleteValue(hKey, L"kingqaquuu");
    if (lRet == ERROR_SUCCESS) {
        printf("delete success!\n");
    }
    else {
        printf("delete fail!\n");
    }
    RegCloseKey(hKey);
}
