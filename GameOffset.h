#pragma once

//********************************************
/* 游戏结构 */
//[PlantsVsZombies.exe + 0x355E0C]
#define GAMEFIXEDADDRESS 0x755E0C
//游戏开始结构
#define GAMESTART 0x868
//阳光基址
#define SUNSHINEOFFSET 0x5578
//栏位信息数组
#define FIELDOFFSET 0x15C
//栏位结构体大小
#define SIZEOFFIELD 0x50
//种植植物call
#define PLANTCALL 0x418D70
//刷僵尸call
#define ZOMBIECALL 0x419a60
//********************************************
//指令
#define SWITCH_INIT 1
#define SWITCH_CHANGESUN 2
#define SWITCH_NOCD 3
#define SWITCH_KILL 4
#define SWITCH_ZOMBIE 5
//********************************************
//共享内存名
#define ShareMemName "PVZTrainer"