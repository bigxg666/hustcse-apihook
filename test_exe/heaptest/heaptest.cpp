// heaptest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <windows.h>
#include <stdio.h>

#define PAGE_SIZE 4096

int main() {
    HANDLE heapHandle = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 5, PAGE_SIZE * 50);
    if (heapHandle == NULL) {
        printf("创建堆失败\n");
        return 1;
    }

    int* ptr = (int*)HeapAlloc(heapHandle, 0, sizeof(int) * 20);
    if (ptr == NULL) {
        printf("堆内存分配失败\n");
        return 1;
    }

    for (int i = 0; i < 20; ++i)
    {
        ptr[i] = i;
    }

    printf("按回车释放第一次内存\n");
    getchar();

    printf("第一次释放堆内存\n\n");
    if (!HeapFree(heapHandle, 0, ptr)) {
        printf("第一次释放堆内存失败\n\n");
        return 1;
    }

    printf("按回车释放第二次内存\n");
    getchar();

    printf("第二次释放堆内存\n\n");
    if (!HeapFree(heapHandle, 0, ptr)) {
        printf("第二次释放堆内存失败\n\n");
        return 1;
    }


    HeapDestroy(heapHandle);

    return 0;
}


