#include <head.h>
extern void regCreateAndSetValue()
{
    HKEY hKey = NULL;
    TCHAR Data[254];
    memset(Data, 0, sizeof(Data));
    wcsncpy_s(Data, TEXT("https://gitee.com/hustsecurity/"), 254);

    size_t lRet = RegCreateKeyEx(HKEY_CURRENT_USER, (LPWSTR)L"LM's_Key", 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet == ERROR_SUCCESS) {
        printf("create successfully!\n");
    }
    else {
        printf("failed to create!\n");
    }

    // 修改注册表键值，没有则创建它
    size_t iLen = wcslen(Data);

    // 设置键值
    lRet = RegSetValueEx(hKey, L"kingqaquuu", 0, REG_SZ, (CONST BYTE*)Data, sizeof(TCHAR) * iLen);
    if (lRet == ERROR_SUCCESS)
    {
        printf("set value successfully!\n");
    }
    else {
        printf("failed to set value!\n");
    }
    RegCloseKey(hKey);
    return;
}