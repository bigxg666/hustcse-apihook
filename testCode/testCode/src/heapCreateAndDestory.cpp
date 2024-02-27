#include <head.h>
extern void heapCreateAndDestory()
{
    printf("Press any key to start HeapCreate!\n");
    getchar();
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 10, PAGE_SIZE * 100);

    int* pArr = (int*)HeapAlloc(hHeap, 0, sizeof(int) * 30);
    //按照如下的规律进行堆的建立
    for (int i = 0; i < 30; ++i)
    {
        pArr[i] = i + 1;
    }
    printf("Successfully created!\n");

    //将建好的堆打印出来
    for (int i = 0; i < 30; ++i)
    {
        if (i % 5 == 0)
            printf_s("\n");
        printf("%3d ", pArr[i]);
    }
    printf_s("\n\n");

    printf("Press any key to start HeapFree!\n");
    getchar();
    HeapFree(hHeap, 0, pArr);

    printf("Press any key to start HeapDestory!\n");
    getchar();
    HeapDestroy(hHeap);
    printf("Successfully destory!\n");
}