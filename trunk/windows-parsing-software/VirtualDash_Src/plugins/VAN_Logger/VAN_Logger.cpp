// VAN_Logger.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "VAN_Logger.h"
#include "LogDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HANDLE hLogThread = INVALID_HANDLE_VALUE;
CLogDlg* dlg = NULL;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//
DWORD WINAPI LogThread(LPVOID lpParameter)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWnd* pParent = (CWnd*)lpParameter;
	int ret;

	OutputDebugString("LogThread started\n");
	

	
	AfxInitRichEdit2();

	ret = dlg->DoModal();
	if(ret == -1)
	{
		OutputDebugString("ERROR : Cannot create LogDlg!\n");
	}
	else if(ret == IDABORT)
	{
		OutputDebugString("ERROR :unexpected LogDlg error!\n");
	}

	

	OutputDebugString("LogThread Exited\n");

	return 0;
}

VAN_LOGGER_API int _stdcall Get_PluginType(void)
{
	return LOGGER_PLUGIN;
}

VAN_LOGGER_API int _stdcall Init_Plugin(void* pParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	dlg = new CLogDlg();
	hLogThread = CreateThread(NULL,0, LogThread, pParent, 0, NULL);
	return 0;
}

VAN_LOGGER_API int _stdcall Close_Plugin(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(hLogThread != INVALID_HANDLE_VALUE)
	{
		OutputDebugString("Exiting Log Thread\n");
		if(dlg)
			dlg->exitDlg();
		WaitForSingleObject(hLogThread, 30000);
		CloseHandle(hLogThread);
		hLogThread = INVALID_HANDLE_VALUE;
	}

	if(dlg)
		delete dlg;
	dlg = NULL;
	
	return 0;
}

VAN_LOGGER_API int _stdcall Process_Raw_VAN_Packet(int addr, uint8* pkt, uint16 len)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(dlg)
		dlg->LogData(addr, (char*)pkt, len);

	return 0;
}

