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
	printf("1.��ʼ��     2.�޸�����\n3.��CD       4.ȫ����ɱ\n5.ˢ��ʬ     ���������˳�...\n");
	printf("****************************************************************\n");

}

int main() {

	int UserInput = 0, ScanfRet = 0;
	BaseTool baseTool;
	while (true) {
		ShowMenu();
		ScanfRet = scanf_s("%d", &UserInput);
		//�����ж�
		if (ScanfRet == EOF) {
			return 0;
		}
		//�ַ�
		switch(UserInput) {
		case SWITCH_INIT:
		{
			if (baseTool._getProcessID() == 0) {
				if (baseTool.FindProcessByName("PlantsVsZombies.exe")) {
					cout << "���̼������" << endl;
					DWORD base = GetBaseAddr(baseTool._getProcessID());
				}				
				else {
					cout << "���̼���ʧ��" << endl;
					break;
				}
			}
			else {
				cout << "�����Ѿ��������" << endl;
			}
			if (baseTool.Inject() == true) {
				cout << "ע�����" << endl;
			}
			else {
				cout << "ע��ʧ��" << endl;
			}
			break;
		}
		case SWITCH_CHANGESUN:
		{
			baseTool.EditSunshine();
			cout << "�޸����" << endl;
			break;
		}
		case SWITCH_NOCD:
		{
			baseTool.NoCoolDown();
			cout << "�������" << endl;
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
	//�����ں˶���

	return 0;
}