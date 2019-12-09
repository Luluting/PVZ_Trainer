#include "BaseTool.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

void ShowMenu() {
	cout << R"(    ____ _    _______        __  _______  ____ )" << endl;
	cout << R"(   / __ \ |  / /__  /       /  |/  / __ \/ __ \)" << endl;
	cout << R"(  / /_/ / | / /  / /       / /|_/ / / / / / / /)" << endl;
	cout << R"( / ____/| |/ /  / /__     / /  / / /_/ / /_/ / )" << endl;
	cout << R"(/_/     |___/  /____/____/_/  /_/\____/_____/  )" << endl;
	cout << R"(                   /_____/                     )" << endl;
	cout << R"(                                               )" << endl;

	printf("****************************************************************\n");
	printf("1.初始化     2.修改阳光\n3.无CD       4.全屏秒杀\n5.刷僵尸     输入其它退出...\n");
	printf("****************************************************************\n");

}

int main() {

	int UserInput = 0, ScanfRet = 0;
	BaseTool baseTool;
	while (true) {
		ShowMenu();
		ScanfRet = scanf_s("%d", &UserInput);
		//结束判断
		if (ScanfRet == EOF) {
			return 0;
		}
		//分发
		switch(UserInput) {
		case SWITCH_INIT:
		{
			if (baseTool._getProcessID() == 0) {
				if (baseTool.FindProcessByName("PlantsVsZombies.exe")) {
					cout << "进程加载完成" << endl;
					DWORD base = GetBaseAddr(baseTool._getProcessID());
				}				
				else {
					cout << "进程加载失败" << endl;
					break;
				}
			}
			else {
				cout << "进程已经加载完毕" << endl;
			}
			if (baseTool.Inject() == true) {
				cout << "注入完成" << endl;
			}
			else {
				cout << "注入失败" << endl;
			}
			break;
		}
		case SWITCH_CHANGESUN:
		{
			baseTool.EditSunshine();
			cout << "修改完成" << endl;
			break;
		}
		case SWITCH_NOCD:
		{
			baseTool.NoCoolDown();
			cout << "操作完成" << endl;
			break;
		}
		case SWITCH_KILL:
		{
			baseTool.KillAll();
			break;
		}
		case SWITCH_ZOMBIE: 
		{
			baseTool.CreateZombie();
			break;
		}
		default: 
		{
			baseTool.Deject();
			return 0;
		}
		}
		Sleep(1000);
		system("cls");
	}
	//清理内核对象

	return 0;
}