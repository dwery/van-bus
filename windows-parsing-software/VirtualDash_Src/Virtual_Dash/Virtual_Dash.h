// Virtual_Dash.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CVirtual_DashApp:
// See Virtual_Dash.cpp for the implementation of this class
//

class CVirtual_DashApp : public CWinApp
{
public:
	CVirtual_DashApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVirtual_DashApp theApp;