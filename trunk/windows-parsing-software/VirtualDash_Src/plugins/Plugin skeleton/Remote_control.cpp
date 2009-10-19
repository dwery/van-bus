// Remote_control.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>

#include "Remote_control.h"

//#include "winamp/wa_ipc.h"
#include "../../common/inc/plugins/plugins_defs.h"

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


REMOTE_CONTROL_API int _stdcall Get_PluginType(void)
{
	return BACKGROUND_PLUGIN;
}

REMOTE_CONTROL_API int _stdcall Init_Plugin(void)
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

	return 0;
}
