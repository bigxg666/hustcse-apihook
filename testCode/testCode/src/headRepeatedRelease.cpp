#include <head.h>
extern void headRepeatedRelease() {

    printf("Press any key to start HeapCreate!\n");
    getchar();
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, PAGE_SIZE * 10, PAGE_SIZE * 100);

    int* pArr = (int*)HeapAlloc(hHeap, 0, sizeof(int) * 30);
    for (int i = 0; i < 30; ++i)
    {
        pArr[i] = i + 1;
    }
    printf("Successfully created!\n");
    for (int i = 0; i < 30; ++i)
    {
        if (i % 5 == 0)
            puts("");
        printf("%3d ", pArr[i]);
    }
    puts(""); puts("");
    printf("Press any key to start the first HeapDestroy!\n");
    getchar();
    HeapDestroy(hHeap);
    printf("Press any key to start the second HeapDestroy!\n");
    getchar();
    HeapDestroy(hHeap);
}