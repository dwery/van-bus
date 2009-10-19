// VirtualDisplay.h : main header file for the VirtualDisplay DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CVirtualDisplayApp
// See VirtualDisplay.cpp for the implementation of this class
//

class CVirtualDisplayApp : public CWinApp
{
public:
	CVirtualDisplayApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};


//Common to all plugins...
int _stdcall  Get_PluginType(void);
/////////////////////////////

//return 0 if no error
int _stdcall Init_Plugin(void* pParent);

//return 0 if no error
int _stdcall Close_Plugin(void);


//return 0 if no error
int _stdcall Process_Message(int addr, unsigned int msgType, void* msg);