// VAN_Logger.h : main header file for the VAN_Logger DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#ifdef VAN_LOGGER_EXPORTS
#define VAN_LOGGER_API __declspec(dllexport)
#else
#define VAN_LOGGER_API __declspec(dllimport)
#endif

#include "../../common/inc/plugins/plugins_defs.h"

//Common to all plugins...
VAN_LOGGER_API int _stdcall  Get_PluginType(void);
/////////////////////////////

//return 0 if no error
VAN_LOGGER_API int _stdcall Init_Plugin(void* pParent);

//return 0 if no error
VAN_LOGGER_API int _stdcall Close_Plugin(void);


VAN_LOGGER_API int _stdcall Process_Raw_VAN_Packet(int addr, uint8* pkt, uint16 len);
