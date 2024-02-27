/*
 * @Author: kingqaquuu
 * @Date: 2024-02-27 10:56:40
 * @FilePath: \hustcse-apihook\testCode\testCode\main.cpp
 */
#include "include/head.h"
using namespace std;
int main() {
	int choice = 0;
	while (1) {
		showMenu();
		scanf("%d", &choice);
		switch (choice){
			case 0:{//退出
				printf("bye!\n");
				break;
			}
			//正常操作
			case 1:{//MessageBoxA
				MessageBoxA(NULL, "I'm MessageBoxA", "I'm MessageBoxA's title", MB_OK);
				system("pause");
				system("cls");
				break;
			}
			case 2:{//MessageBoxW
				MessageBoxW(NULL, L"I'm MessageBoxW", L"I'm MessageBoxW's title", MB_OK);
				system("pause");
				system("cls");
				break;
			}
			case 3:{//创造并写文件
				writeFileString();
				system("pause");
				system("cls");
				break;
			}
			case 4:{//读文件
				readFileString();
				system("pause");
				system("cls");
				break;
			}
			case 5:{//堆创建和释放
				heapCreateAndDestory();
				system("pause");
				system("cls");
				break;
			}
			case 6:{//注册表创建和设置值
				regCreateAndSetValue();
				system("pause");
				system("cls");
				break;
			}
			case 7:{//注册表打开和删除值
				regOpenAndDelValue();
				system("pause");
				system("cls");
				break;
			}
			case 8:{//接收数据
				recvData();
				system("pause");
				system("cls");
				break;
			}
			case 9:{//发送数据
				sendData();
				system("pause");
				system("cls");
				break;
			}
			//恶意操作
			case 10:{//修改程序
				modifyExProgram();
				system("pause");
				system("cls");
				break;
			}
			case 11:{//自身复制
				selfReplication();
				system("pause");
				system("cls");
				break;
			}
			case 12:{//打开另一个文件夹
				openAnotherFolder();
				system("pause");
				system("cls");
				break;
			}
			case 13:{//释放重复堆
				headRepeatedRelease();
				system("pause");
				system("cls");
				break;
			}
			case 14:{//修改注册表
				modifyStartupRegistry();
				system("pause");
				system("cls");
				break;
				}
			default:{
				system("pause");
				system("cls");
				break;
			}
		}
		if (choice == 0) break;
	}
	return 0;
}