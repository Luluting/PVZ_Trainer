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
	//修改
	void EditSunshine();
	void NoCoolDown();
	void KillAll();
	void CreateZombie();
	//关句柄
	~BaseTool() {
		CloseHandle(ProcessHandle);
		CloseHandle(NoCDThread);
		CloseHandle(g_hShareMemory);
	}
};

//线程函数
DWORD WINAPI NoCDThreadFunc(LPVOID p) {
	while (true) {
		DWORD Address = 0;
		ReadProcessMemory((HANDLE)p, (LPCVOID)GAMEFIXEDADDRESS, (LPVOID)& Address, sizeof(DWORD), NULL);
		ReadProcessMemory((HANDLE)p, (LPCVOID)(Address + GAMESTART), (LPVOID)& Address, sizeof(DWORD), NULL);
		if (Address == NULL) {
			cout << "请先进入游戏" << endl;
			return 0;
		}
		ReadProcessMemory((HANDLE)p, (LPCVOID)(Address + FIELDOFFSET), (LPVOID)& Address, sizeof(DWORD), NULL);
		//这里忘了记录栏位数目的地址 就直接写上限了 不知道栏位没开全会不会报错
		for (int i = 0; i < 10; i++) {
			DWORD tempAdd = (DWORD)((char*)Address + i * 0x50 + 0x28 + 0x24);
			DWORD CoolValue = 0x100000;
			WriteProcessMemory((HANDLE)p, (LPVOID)(tempAdd), (LPVOID)& CoolValue, sizeof(DWORD), NULL);
		}
	}
	return 0;
}

//BaseTool实现

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
//寻找进程
//参数: const char*
//返回值: DWORD
/******************************************************************************************/
DWORD BaseTool::FindProcessByName(const char* pName) {
	//拍摄进程快照
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//遍历进程
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapShot, &pe32);
	//查找PVZ进程返回进程句柄
	do
	{
		if (strcmp(pe32.szExeFile, pName) == 0) {
			CloseHandle(hSnapShot);
			ProcessID = pe32.th32ProcessID;
			ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pe32.th32ProcessID);
			return 1;
		}
	} while (Process32Next(hSnapShot, &pe32));
	//未找到进程
	CloseHandle(hSnapShot);
	return 0;
}
/******************************************************************************************/
//创建共享内存后注入功能DLL
//参数: void
//返回值: int
/******************************************************************************************/
#define DLLNAME "\\ProcessCom.dll"

int BaseTool::Inject() {
	//创建共享内存 用于和DLL通信
	g_hShareMemory = CreateFileMapping(NULL, NULL, PAGE_EXECUTE_READWRITE, 0, 0x10000, ShareMemName);
	if (g_hShareMemory == NULL || g_hShareMemory == INVALID_HANDLE_VALUE) {
		return false;
	}
	g_pShareMemory = MapViewOfFile(g_hShareMemory, FILE_MAP_ALL_ACCESS, 0, 0, NULL);
	if (g_pShareMemory == NULL) {
		return false;
	}
	cout << "共享内存创建成功" << endl;

	//远程线程注入DLL
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
//卸载
//参数: void
//返回值: int
/******************************************************************************************/
#define FDLLNAME "ProcessCom.dll"
int BaseTool::Deject() {
	//搜索模块地址
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (hSnapShot == NULL) {
		return false;
	}
	MODULEENTRY32 me32 = { 0 };
	me32.dwSize = sizeof(MODULEENTRY32);
	Module32First(hSnapShot, &me32);
	//查找PVZ进程返回进程句柄
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
	 
	//远程线程卸载DLL
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
//修改阳光
//参数: void
//返回值: void
/******************************************************************************************/
void BaseTool::EditSunshine() {
	if (ProcessHandle == NULL) {
		cout << "请先初始化!\n" << endl;
		return;
	}
	int num;
	cout << "输入修改值:" << endl;
	cin >> num;
	DWORD Address = 0;
	ReadProcessMemory(ProcessHandle, (LPCVOID)GAMEFIXEDADDRESS, (LPVOID) &Address, sizeof(DWORD), NULL);
	ReadProcessMemory(ProcessHandle, (LPCVOID)(Address + GAMESTART), (LPVOID) &Address, sizeof(DWORD), NULL);
	WriteProcessMemory(ProcessHandle, (LPVOID)(Address + SUNSHINEOFFSET), (LPCVOID) &num, sizeof(DWORD), NULL);
	return;
}
/******************************************************************************************/
//无冷却功能
//参数: void
//返回值: void
/******************************************************************************************/
void BaseTool::NoCoolDown() {
	if (ProcessHandle == NULL) {
		cout << "请先初始化!\n" << endl;
		return;
	}
	if (NoCDThread != NULL) {
		cout << "之前已经开启 现在帮您关闭" << endl;
		TerminateThread(NoCDThread,0);
		NoCDThread = NULL;
		return;
	}
	NoCDThread = CreateThread(NULL, 0, NoCDThreadFunc, (LPVOID)ProcessHandle, NULL, NULL);
}
/******************************************************************************************/
//全屏秒杀
//参数: void
//返回值: void
/******************************************************************************************/
void BaseTool::KillAll() {
	*(int*)g_pShareMemory = 4;
}
/******************************************************************************************/
//刷僵尸
//参数: void
//返回值: void
/******************************************************************************************/
void BaseTool::CreateZombie() {
	int row = 0, zombieID = 0, num = 0;
	cout << "选择要刷的行数:" << endl;
	cin >> row;
	cout << "选择要刷的僵尸ID:" << endl;
	cin >> zombieID;
	cout << "选择要刷的数量:" << endl;
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