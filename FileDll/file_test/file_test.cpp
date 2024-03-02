#include <windows.h>
#include <iostream>

int main() {
    // 测试 CreateFile 函数
    HANDLE hFile = CreateFile(TEXT("C:\\Users\\86130\\desktop\\test.txt"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create file. Error code: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Test program: File created successfully." << std::endl;

    // 测试写文件
    char buffer[100] = "Hello, World!";
    DWORD bytesWritten;
    if (!WriteFile(hFile, buffer, sizeof(buffer), &bytesWritten, NULL)) {
        std::cerr << "Failed to write to file. Error code: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }
    std::cout << "Test program: Data written to file." << std::endl;

    // 读取文件内容
    char readBuffer[100] = { 0 };
    DWORD bytesRead;
    if (!ReadFile(hFile, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
        std::cerr << "Failed to read from file. Error code: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }
    std::cout << "Test program: Data read from file: " << readBuffer << std::endl;

    // 关闭文件
    if (!CloseHandle(hFile)) {
        std::cerr << "Failed to close file handle. Error code: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Test program: File handle closed." << std::endl;


    //复制文件
    CopyFile(L"C:\\Users\\86130\\desktop\\test.txt", L"a.txt", FALSE);

    //用CreateFile来打开a.txt文件
    HANDLE hFileA = CreateFile(TEXT("a.txt"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    //测试操作范围是否有多个文件夹，与前面的文件夹路径要不同
    if (!ReadFile(hFileA, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
        std::cerr << "Failed to read from file. Error code: " << GetLastError() << std::endl;
        CloseHandle(hFileA);
        return 1;
    }

    /*
    if (!WriteFile(hFileA, buffer, sizeof(buffer), &bytesWritten, NULL)) {
        std::cerr << "Failed to write to file. Error code: " << GetLastError() << std::endl;
        CloseHandle(hFileA);
        return 1;
    }
    */

    // 删除文件
    if (!DeleteFile(TEXT("C:\\Users\\86130\\desktop\\test.txt"))) {
        std::cerr << "Failed to delete file. Error code: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Test program: File deleted successfully." << std::endl;

    return 0;
}