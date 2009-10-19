// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the REMOTE_CONTROL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// REMOTE_CONTROL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef REMOTE_CONTROL_EXPORTS
#define REMOTE_CONTROL_API __declspec(dllexport)
#else
#define REMOTE_CONTROL_API __declspec(dllimport)
#endif

#define NO_PLAYER		0
#define WINAMP_PLAYER	1
#define WMP_PLAYER		2
#define SONIC_PLAYER	3

#define PLAYER_PLAY			0
#define PLAYER_PAUSE		1
#define PLAYER_STOP			2
#define PLAYER_NEXT			3
#define PLAYER_PREV			4
#define PLAYER_VOL_UP		5
#define PLAYER_VOL_DOWN		6

#define PLAYER_ERROR		0
#define PLAYER_PAUSED		1
#define PLAYER_PLAYING		2
#define PLAYER_STOPPED		3

//Common to all plugins...
REMOTE_CONTROL_API int _stdcall  Get_PluginType(void);
/////////////////////////////

//return 0 if no error
REMOTE_CONTROL_API int _stdcall Init_Plugin(void* pParent);

//return 0 if no error
REMOTE_CONTROL_API int _stdcall Close_Plugin(void);


//return 0 if no error
REMOTE_CONTROL_API int _stdcall Process_Message(int addr, unsigned int msgType, void* msg);
