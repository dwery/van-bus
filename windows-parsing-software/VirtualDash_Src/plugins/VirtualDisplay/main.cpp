#include "stdafx.h"
#include "VirtualDisplay.h"
#include "VirtualDisplayDlg.h"

#include <stdio.h>
#include "../../common/inc/plugins/plugins_defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CVirtualDisplayDlg* dlg;
HANDLE hRenderThread = INVALID_HANDLE_VALUE;


DWORD WINAPI RenderThread(LPVOID lpParameter)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWnd* pParent = (CWnd*)lpParameter;
	int ret;

	OutputDebugString("RenderThread started\n");

	ret = dlg->DoModal();
	if(ret == -1)
	{
		OutputDebugString("ERROR : Cannot create RenderDlg!\n");
	}
	else if(ret == IDABORT)
	{
		OutputDebugString("ERROR :unexpected RenderDlg error!\n");
	}

	OutputDebugString("RenderThread Exited\n");

	return 0;
}

//Common to all plugins...
int _stdcall  Get_PluginType(void)
{
	return RENDERER_PLUGIN;
}
/////////////////////////////

//return 0 if no error
int _stdcall Init_Plugin(void* pParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	dlg = new CVirtualDisplayDlg();
	hRenderThread = CreateThread(NULL,0, RenderThread, pParent, 0, NULL);
	return 0;
}

//return 0 if no error
int _stdcall Close_Plugin(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(hRenderThread != INVALID_HANDLE_VALUE)
	{
		OutputDebugString("Exiting Render Thread\n");
		if(dlg)
			dlg->exitDlg();
		WaitForSingleObject(hRenderThread, 30000);
		CloseHandle(hRenderThread);
		hRenderThread = INVALID_HANDLE_VALUE;
	}

	if(dlg)
		delete dlg;
	dlg = NULL;

	return 0;
}


//return 0 if no error
int _stdcall Process_Message(int addr, unsigned int msgType, void* msg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(dlg)
		return dlg->ProcessMessage(addr, msgType, msg);
	
	return 1;
}