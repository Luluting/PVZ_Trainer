#pragma once
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <TlHelp32.h>
#include <string.h>
#include <iostream>
#include "GameOffset.h"


using namespace std;

class BaseTool {
private:
	DWORD ProcessID = 0;
	HANDLE ProcessHandle = 0;
	HANDLE NoCDThread = 0;
	HANDLE g_hShareMemory = 0;
	LPVOID g_pShareMemory = 0;
	HANDLE moduleHandle = 0;
public:
	DWORD _getProcessID();
	HANDLE _getProcessHandle();
	void __stdcall _MyHandle(LPVOID i);
	DWORD FindProcessByName(const char* pName);
	int Inject();
	int Deject();
	//�޸�
	void EditSunshine();
	void NoCoolDown();
	void KillAll();
	void CreateZombie();
	//�ؾ��
	~BaseTool() {
		CloseHandle(ProcessHandle);
		CloseHandle(NoCDThread);
		CloseHandle(g_hShareMemory);
	}
};

//�̺߳���
DWORD WINAPI NoCDThreadFunc(LPVOID p) {
	while (true) {
		DWORD Address = 0;
		ReadProcessMemory((HANDLE)p, (LPCVOID)GAMEFIXEDADDRESS, (LPVOID)& Address, sizeof(DWORD), NULL);
		ReadProcessMemory((HANDLE)p, (LPCVOID)(Address + GAMESTART), (LPVOID)& Address, sizeof(DWORD), NULL);
		if (Address == NULL) {
			cout << "���Ƚ�����Ϸ" << endl;
			return 0;
		}
		ReadProcessMemory((HANDLE)p, (LPCVOID)(Address + FIELDOFFSET), (LPVOID)& Address, sizeof(DWORD), NULL);
		//�������˼�¼��λ��Ŀ�ĵ�ַ ��ֱ��д������ ��֪����λû��ȫ�᲻�ᱨ��
		for (int i = 0; i < 10; i++) {
			DWORD tempAdd = (DWORD)((char*)Address + i * 0x50 + 0x28 + 0x24);
			DWORD CoolValue = 0x100000;
			WriteProcessMemory((HANDLE)p, (LPVOID)(tempAdd), (LPVOID)& CoolValue, sizeof(DWORD), NULL);
		}
	}
	return 0;
}

//BaseToolʵ��

DWORD BaseTool::_getProcessID() {
	return ProcessID;
}
HANDLE BaseTool::_getProcessHandle() {
	return ProcessHandle;
}

void __stdcall BaseTool::_MyHandle(LPVOID i) {
	printf("%d\n", (int)i);
}

/******************************************************************************************/
//Ѱ�ҽ���
//����: const char*
//����ֵ: DWORD
/******************************************************************************************/
DWORD BaseTool::FindProcessByName(const char* pName) {
	//������̿���
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//��������
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapShot, &pe32);
	//����PVZ���̷��ؽ��̾��
	do
	{
		if (strcmp(pe32.szExeFile, pName) == 0) {
			CloseHandle(hSnapShot);
			ProcessID = pe32.th32ProcessID;
			ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pe32.th32ProcessID);
			return 1;
		}
	} while (Process32Next(hSnapShot, &pe32));
	//δ�ҵ�����
	CloseHandle(hSnapShot);
	return 0;
}
/******************************************************************************************/
//���������ڴ��ע�빦��DLL
//����: void
//����ֵ: int
/******************************************************************************************/
#define DLLNAME "\\ProcessCom.dll"

int BaseTool::Inject() {
	//���������ڴ� ���ں�DLLͨ��
	g_hShareMemory = CreateFileMapping(NULL, NULL, PAGE_EXECUTE_READWRITE, 0, 0x10000, ShareMemName);
	if (g_hShareMemory == NULL || g_hShareMemory == INVALID_HANDLE_VALUE) {
		return false;
	}
	g_pShareMemory = MapViewOfFile(g_hShareMemory, FILE_MAP_ALL_ACCESS, 0, 0, NULL);
	if (g_pShareMemory == NULL) {
		return false;
	}
	cout << "�����ڴ洴���ɹ�" << endl;

	//Զ���߳�ע��DLL
	if (ProcessHandle == NULL) {
		if (!FindProcessByName("PlantsVsZombies.exe")) {
			return -1;
		}
	}
	LPVOID VirtualAddress = VirtualAllocEx(ProcessHandle, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	char szPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, szPath);
	strcat_s((char*)szPath, MAX_PATH, DLLNAME);
	WriteProcessMemory(ProcessHandle, VirtualAddress, (LPCVOID)szPath, MAX_PATH, NULL);
	HANDLE PVZremote = CreateRemoteThread(
		ProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, (LPVOID)VirtualAddress, 0, NULL
	);
	WaitForSingleObject(PVZremote, INFINITE);
	DWORD nExitCode = 0;
	GetExitCodeThread(PVZremote, &nExitCode);
	if (nExitCode == 0) {
		cout << "Inject dll failed" << endl;
		return false;
	}
	VirtualFreeEx(ProcessHandle, VirtualAddress, MAX_PATH, MEM_DECOMMIT);
	CloseHandle(PVZremote);
	return true;
}
/******************************************************************************************/
//ж��
//����: void
//����ֵ: int
/******************************************************************************************/
#define FDLLNAME "ProcessCom.dll"
int BaseTool::Deject() {
	//����ģ���ַ
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (hSnapShot == NULL) {
		return false;
	}
	MODULEENTRY32 me32 = { 0 };
	me32.dwSize = sizeof(MODULEENTRY32);
	Module32First(hSnapShot, &me32);
	//����PVZ���̷��ؽ��̾��
	do
	{
		if (strcmp(me32.szModule, FDLLNAME) == 0) {
			CloseHandle(hSnapShot);
			moduleHandle = me32.hModule;
		}
	} while (Module32Next(hSnapShot, &me32));
	if (moduleHandle == NULL) {
		return false;
	}
	 
	//Զ���߳�ж��DLL
	if (ProcessHandle == NULL) {
		if (!FindProcessByName("PlantsVsZombies.exe")) {
			return -1;
		}
	}
	LPVOID VirtualAddress = VirtualAllocEx(ProcessHandle, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	int szPath[MAX_PATH] = { 0 };
	int* module = (int*)&moduleHandle;
	szPath[0] = *module;
	WriteProcessMemory(ProcessHandle, VirtualAddress, (LPCVOID)&szPath, 4, NULL);
	HANDLE PVZremote = CreateRemoteThread(
		ProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, (LPVOID)VirtualAddress, 0, NULL
	);
}
/******************************************************************************************/
//�޸�����
//����: void
//����ֵ: void
/******************************************************************************************/
void BaseTool::EditSunshine() {
	if (ProcessHandle == NULL) {
		cout << "���ȳ�ʼ��!\n" << endl;
		return;
	}
	int num;
	cout << "�����޸�ֵ:" << endl;
	cin >> num;
	DWORD Address = 0;
	ReadProcessMemory(ProcessHandle, (LPCVOID)GAMEFIXEDADDRESS, (LPVOID) &Address, sizeof(DWORD), NULL);
	ReadProcessMemory(ProcessHandle, (LPCVOID)(Address + GAMESTART), (LPVOID) &Address, sizeof(DWORD), NULL);
	WriteProcessMemory(ProcessHandle, (LPVOID)(Address + SUNSHINEOFFSET), (LPCVOID) &num, sizeof(DWORD), NULL);
	return;
}
/******************************************************************************************/
//����ȴ����
//����: void
//����ֵ: void
/******************************************************************************************/
void BaseTool::NoCoolDown() {
	if (ProcessHandle == NULL) {
		cout << "���ȳ�ʼ��!\n" << endl;
		return;
	}
	if (NoCDThread != NULL) {
		cout << "֮ǰ�Ѿ����� ���ڰ����ر�" << endl;
		TerminateThread(NoCDThread,0);
		NoCDThread = NULL;
		return;
	}
	NoCDThread = CreateThread(NULL, 0, NoCDThreadFunc, (LPVOID)ProcessHandle, NULL, NULL);
}
/******************************************************************************************/
//ȫ����ɱ
//����: void
//����ֵ: void
/******************************************************************************************/
void BaseTool::KillAll() {
	*(int*)g_pShareMemory = 4;
}
/******************************************************************************************/
//ˢ��ʬ
//����: void
//����ֵ: void
/******************************************************************************************/
void BaseTool::CreateZombie() {
	int row = 0, zombieID = 0, num = 0;
	cout << "ѡ��Ҫˢ������:" << endl;
	cin >> row;
	cout << "ѡ��Ҫˢ�Ľ�ʬID:" << endl;
	cin >> zombieID;
	cout << "ѡ��Ҫˢ������:" << endl;
	cin >> num;
	*((int*)(g_pShareMemory)+1) = row;   
	*((int*)(g_pShareMemory)+2) = zombieID;
	*((int*)(g_pShareMemory)+3) = num;
	*(int*)g_pShareMemory = SWITCH_ZOMBIE;
}

DWORD GetBaseAddr(DWORD pid)
{
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	MODULEENTRY32 me;
	me.dwSize = sizeof(MODULEENTRY32);
	Module32First(hModuleSnap, &me);
	DWORD dwBaseAddr;
	dwBaseAddr = (DWORD)me.modBaseAddr;
	CloseHandle(hModuleSnap);
	return dwBaseAddr;
}