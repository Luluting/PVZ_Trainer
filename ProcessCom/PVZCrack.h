#pragma once
#include <Windows.h>
#include "..//GameOffset.h"

LPVOID g_pShareMemory;

class DllTool {
private:
	HANDLE hThread;
	HANDLE hShareMemory;
	
public:
	void init();
	void KillAll();
	void CreateZombie();
};


#pragma region Thread
//Ïß³Ìº¯Êý
DWORD ScanThread(LPVOID lpParam) {
	DllTool myTool;
	while (true) {
		int UserSwitch = *(int*)g_pShareMemory;

		switch (UserSwitch) {
		case SWITCH_KILL:
		{
			myTool.KillAll();
			break;
		}
		case SWITCH_ZOMBIE:
		{
			myTool.CreateZombie();
			break;
		}
		}
		*(int*)g_pShareMemory = 0;
		Sleep(1000);
	}
}
#pragma endregion

#pragma region Mod

void DllTool::init() {
	hShareMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, ShareMemName);
	if (hShareMemory == NULL) {
		MessageBox(NULL, "No ShareMem Handle", "PVZTrainer", MB_OK);
	}
	g_pShareMemory = MapViewOfFile(hShareMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0x10000);
	if (g_pShareMemory == NULL) {
		MessageBox(NULL, "No ShareMem Pointer", "PVZTrainer", MB_OK);
	}
}

void DllTool::KillAll() {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 9; j++) {
			__asm {
				pushad
				pushfd
				push - 1
				push 2
				mov eax, i
				push j
				mov ecx, dword ptr ds : [GAMEFIXEDADDRESS]
				mov ecx, dword ptr ds : [ecx + GAMESTART]
				push ecx
				mov ecx, PLANTCALL
				call ecx
				popfd
				popad
			}
		}
	}
}

void DllTool::CreateZombie() {
	int row = 0, zombieID = 0, num = 0;
	row = *((int*)(g_pShareMemory)+1);
	zombieID = *((int*)(g_pShareMemory)+2);
	num = *((int*)(g_pShareMemory)+3);
	while (num > 0) {
		num--;
		__asm {
			pushad
			pushfd
			push row
			push zombieID
			mov edi, dword ptr ds : [GAMEFIXEDADDRESS]
			mov eax, dword ptr ds : [edi + GAMESTART]
			mov edi, ZOMBIECALL
			call edi
			popfd
			popad
		}
	};
}
#pragma endregion
