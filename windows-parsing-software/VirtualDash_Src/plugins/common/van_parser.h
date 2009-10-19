#ifndef _VAN_PARSER_H_
#define _VAN_PARSER_H_

#include "../../common/inc/plugins/plugins_defs.h"
#include "../../common/inc/plugins/parser_plugins_defs.h"

uint8 GetBCD(uint8 bcd);
bool isHex(uint8 c);
uint8 GetHex(uint8* pkt);

int ParseVANPacket(int addr, uint8* pkt, uint16 len, CParserPlugin* ptr);

#endif //_VAN_PARSER_H_