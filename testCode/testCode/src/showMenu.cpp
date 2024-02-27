#include <head.h>
using namespace std;
extern void showMenu(){
    printf("--------------------------------------please select an option---------------------------------------\n\n");
    printf("------Normal operation------:\n");
    printf("1.MessageBoxA                 2.MessageBoxW              3.writeFileString          4.readFileString\n");
    printf("5.heapCreateAndDestory        6.regCreateAndSetValue     7.regOpenAndDelValue       8.socketRecvData\n");
    printf("9.socketSendData\n");
    printf("\n-----Malicious operation----:\n");
    printf("10.File_Operation_1 Modifying executable program \n");
    printf("11.File_Operation_2 selfReplication\n");
    printf("12.File_Operation_3 openAnotherFolder\n");
    printf("13.Heap_Operation heapRepeatedRelease\n");
    printf("14.Registry_Operation modifyStartupRegistry\n");
    printf("\n----------------------------------------------------------------------------------------------------\n\n");
    printf("Your Choice:");
}
