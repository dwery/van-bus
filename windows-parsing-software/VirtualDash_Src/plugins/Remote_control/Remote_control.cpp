// Remote_control.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>

#include "Remote_control.h"

#include "winamp/wa_ipc.h"
#include "../../common/inc/plugins/plugins_defs.h"

uint8 old_hu_info_type = RADIO_INFOS_MSG;

int playerID = NO_PLAYER;
char KeyTab[2][7] = {		{'x',			'c',		'v',		'b',		'z',		0, 0},
							{'p',			'p',		's',		'f',		'b',		0, 0}};	

UINT ModifersTab[2][7] = {	{0,				0,			0,			0,			0,			0, 0},
							{VK_CONTROL,	VK_CONTROL, VK_CONTROL, VK_CONTROL, VK_CONTROL, 0, 0}};

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
    return TRUE;
}

void SendKey(HWND hwnd, int action)
{
    short key;
    LPARAM scancode;
	
	if(playerID == NO_PLAYER)
		return;
	
	if(KeyTab[playerID-1][action] == 0)
		return;

	if(ModifersTab[playerID-1][action])
	{
		/*scancode = MapVirtualKey(ModifersTab[playerID-1][action], 0);
		scancode <<= 16;//scan code are from 16-23 in lparam
		scancode |= 1;//since repeat count is one
		PostMessage(hwnd, WM_KEYDOWN, ModifersTab[playerID-1][action], scancode);*/

		keybd_event(ModifersTab[playerID-1][action],0,0,0);
	}

	Sleep(10);
	key = VkKeyScan(KeyTab[playerID-1][action]);
	scancode = MapVirtualKey(key, 0);
	scancode <<= 16;//scan code are from 16-23 in lparam
	scancode |= 1;//since repeat count is one

    PostMessage(hwnd, WM_KEYDOWN, key, scancode);
	//Sleep(10);
	//PostMessage(hwnd, WM_CHAR, key, scancode);
    Sleep(10);

	scancode |= 0xC0000000;
	PostMessage(hwnd, WM_KEYUP, key, scancode);

	Sleep(10);

	if(ModifersTab[playerID-1][action])
	{
		/*scancode = MapVirtualKey(ModifersTab[playerID-1][action], 0);
		scancode <<= 16;//scan code are from 16-23 in lparam
		scancode |= 1;//since repeat count is one
		scancode |= 0xC0000000;
		PostMessage(hwnd, WM_KEYUP, ModifersTab[playerID-1][action], scancode);*/
		keybd_event(ModifersTab[playerID-1][action],0,KEYEVENTF_KEYUP,0);
	}
}

HWND FindPlayer() 
{
    HWND hwnd;

	//WMPlayerApp
	//WMPAppHost
	hwnd = FindWindow("WMPlayerApp", NULL);
    if(hwnd != 0) 
	{
		hwnd = FindWindowEx(hwnd, NULL, "WMPAppHost", NULL);

		if(hwnd != 0) 
		{
			playerID = WMP_PLAYER;
			OutputDebugString("Remote_Control : Found WMP\n");
			return hwnd;
		}
    }

	hwnd = FindWindow("Winamp v1.x", NULL);
    if(hwnd != 0) 
	{
        playerID = WINAMP_PLAYER;
		OutputDebugString("Remote_Control : Found Winamp\n");
        return hwnd;
    }


/*

	hwnd = FindWindow("Sonique Window Class", NULL);
    if (hwnd != 0) 
	{
        playerID = SONIC_PLAYER;
        return hwnd;
    }

	hwnd = FindWindow("Studio", NULL);
    if (hwnd != 0)
	{
        playerID = 3;
        return hwnd;
    }
*/
    
    playerID = NO_PLAYER;

    return 0;
}

uint8 GetPlayerState(void)
{
	HWND hwnd;

	hwnd = FindPlayer();
	if(hwnd == NULL)
		return PLAYER_ERROR;

	if(playerID == WINAMP_PLAYER)
	{
		int res = SendMessage(hwnd, WM_WA_IPC, 0, IPC_ISPLAYING);
		if(res == 1)
			return PLAYER_PLAYING;

		if(res == 3)
			return PLAYER_PAUSED;

		if(res == 0)
			return PLAYER_STOPPED;
	}

	return PLAYER_ERROR;
}

REMOTE_CONTROL_API int _stdcall Get_PluginType(void)
{
	return BACKGROUND_PLUGIN;
}

REMOTE_CONTROL_API int _stdcall Init_Plugin(void* pParent)
{
	return 0;
}

REMOTE_CONTROL_API int _stdcall Close_Plugin(void)
{
	return 0;
}

REMOTE_CONTROL_API int _stdcall Process_Message(int addr, unsigned int msgType, void* msg)
{
	
	if(addr == REMOTE_CONTROL_ADDR)
	{
		REMOTE_CONTROL_MSG_T* evt = (REMOTE_CONTROL_MSG_T*)msg;

		if(old_hu_info_type != CDC_INFOS_MSG)
			return 0; // we are not interrested to remote control messages if we are not in CDC mode

		OutputDebugString("Remote_Control : Process_Message\n");
		char deb[100];
		sprintf(deb,"wheel : 0x%02x\n", evt->wheel);
		OutputDebugString(deb);

		if(evt->controls & CONTROL_NEXT)
		{
			OutputDebugString("Play next\n");
			SendKey(FindPlayer(), PLAYER_NEXT);
		}

		if(evt->controls & CONTROL_PREV)
		{
			OutputDebugString("Play prev\n");
			SendKey(FindPlayer(), PLAYER_PREV);
		}
	}
	else if(addr == HEAD_UNIT_ADDR)
	{
		if(old_hu_info_type == msgType) // nothing changed
			return 0;

		old_hu_info_type = msgType;
		uint8 player_state;
		player_state = GetPlayerState();
		
		char deb[100];
		sprintf(deb,"player_state : %u\n", player_state);
		OutputDebugString(deb);
		if(msgType != CDC_INFOS_MSG)
		{
			if(player_state == PLAYER_PLAYING)
			{
				OutputDebugString("Remote_control : Player is playing : pausing it.\n");
				SendKey(FindPlayer(), PLAYER_PAUSE);
			}
		}
		else
		{
			if(player_state == PLAYER_PAUSED)
			{
				OutputDebugString("Remote_control : Player is paused : resuming it.\n");
				SendKey(FindPlayer(), PLAYER_PLAY);
			}
			else if(player_state == PLAYER_STOPPED)
			{
				OutputDebugString("Remote_control : Player is stopped : starting to play.\n");
				SendKey(FindPlayer(), PLAYER_PLAY);
			}
		}
		
	}

	return 0;
}
