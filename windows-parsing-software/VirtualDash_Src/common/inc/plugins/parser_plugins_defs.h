#ifndef _PARSER_PLUGINS_DEFS_H_
#define _PARSER_PLUGINS_DEFS_H_

#include "../types.h"

//callbacks functions
typedef void (_stdcall *TFdeliverMsgToPlugins)(
	int addr, int msg_type, void* msg
);

typedef void (_stdcall *TFdeliverRAWPacketToPlugins)(
	int addr, uint8* pkt, uint16 len
);

class CParserPlugin
{
public:
	virtual ~CParserPlugin(void) {};
	virtual void deliverMsgToPlugins(int addr, int msg_type, void* msg) = 0;
	virtual void deliverRAWPacketToPlugins(int addr, uint8* pkt, uint16 len) = 0;

};




typedef void (_stdcall *TFInit_Parser)(
	CParserPlugin* callbacks
);

typedef void (_stdcall *TFClose_Parser)(
	void
);

typedef void (_stdcall *TFOnNewByte)(
	uint8 c
);
#endif //_PARSER_PLUGINS_DEFS_H_
