// Raw_VAN_Parser.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "stdio.h"

#include "../../common/inc/plugins/plugins_defs.h"
#include "../../common/inc/plugins/parser_plugins_defs.h"

#include "../common/van_parser.h"


CParserPlugin* ptr = NULL;
uint8 pkt[255];
uint8 raw_pkt[156];
uint16 raw_len = 0;
uint16 i;
int addr;

void HewToAscii(uint8* hex, uint16 len, uint8* txt)
{
	int l;
	uint8 tmp;
	for(l=0;l<len;l++)
	{
		tmp = (hex[l]>>4)&0x0F;
		if((tmp >= 0) && (tmp <= 9))
			txt[2*l] = tmp + 0x30;
		else
			txt[2*l] = tmp - 0x0A + 'A';

		tmp = (hex[l])&0x0F;
		if((tmp >= 0) && (tmp <= 9))
			txt[2*l+1] = tmp + 0x30;
		else
			txt[2*l+1] = tmp - 0x0A + 'A';
	}
	txt[2*l] = 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


__declspec(dllexport) void _stdcall Init_Parser(CParserPlugin* callbacks)
{
	OutputDebugString("Raw_VAN_Parser : Init_Parser\n");
	i = 0;
	raw_len = 0;
	ptr = callbacks;
}

__declspec(dllexport) void _stdcall Close_Parser()
{
	ptr = NULL;
}


__declspec(dllexport) void _stdcall OnNewByte(uint8 c)
{
	if(ptr == NULL)
		return;

	raw_pkt[i] = c;
	++i;

	if(i == 4) // we have a complete header
	{
		if(c != 0)
		{
			OutputDebugString("Parser ERROR : Invalid packet : zero not found!\n");
			i=0;
			return;
		}
		addr = (((uint16)(raw_pkt[0]))<<4) | ((((uint16)(raw_pkt[1]))>>4)&0x000F);
		if(addr == 0)
		{
			OutputDebugString("Parser ERROR : Invalid packet : null addr found!\n");
			i=0;
			return;
		}

		raw_len = raw_pkt[2];	
		if(raw_len == 0)
		{
			OutputDebugString("Parser ERROR : Invalid packet : zero length!\n");
			i=0;
			raw_len = 0;
			return;
		}
		if(raw_len > 40)
		{
			OutputDebugString("Parser ERROR : Invalid packet : length too big!\n");
			i=0;
			raw_len = 0;
			return;
		}

		if(!(raw_pkt[1]&0x08)) // this bit should always be 1
		{
			OutputDebugString("Parser ERROR : Invalid packet :flag not found!\n");
			i=0;
			raw_len = 0;
			return;
		}
	}
	else if(raw_len)
	{
		if(i == (raw_len + 4))
		{
			HewToAscii(raw_pkt + 4, raw_len-1, pkt+8);
			pkt[0] = (raw_pkt[0]>>4)&0x0F;
			if((pkt[0] >= 0) && (pkt[0] <= 9))
				pkt[0] = pkt[0] + 0x30;
			else
				pkt[0] = pkt[0] - 0x0A + 'A';

			pkt[1] = (raw_pkt[0])&0x0F;
			if((pkt[1] >= 0) && (pkt[1] <= 9))
				pkt[1] = pkt[1] + 0x30;
			else
				pkt[1] = pkt[1] - 0x0A + 'A';

			pkt[2] = (raw_pkt[1]>>4)&0x0F;
			if((pkt[2] >= 0) && (pkt[2] <= 9))
				pkt[2] = pkt[2] + 0x30;
			else
				pkt[2] = pkt[2] - 0x0A + 'A';
			pkt[3] = ' ';

			if(raw_pkt[1]&0x02)
				pkt[4] = 'R';
			else
				pkt[4] = 'W';

			if(raw_pkt[1]&0x04)
				pkt[5] = 'A';
			else
				pkt[5] = '-';

			if(raw_pkt[1]&0x01)
				pkt[6] = 'T';
			else
				pkt[6] = '-';

			pkt[7] = ' ';

			if(raw_pkt[i-1] == 0xEE)
				pkt[2*raw_len + 7] = '-';
			else if(raw_pkt[i-1] == 0xEF)
				pkt[2*raw_len + 7] = 'A';
			else
			{
				OutputDebugString("Parser ERROR : Invalid packet : End flag not found!\n");
				i=0;
				raw_len = 0;
				return;
			}

			pkt[2*raw_len + 6] = ' ';
			pkt[2*raw_len + 7 + 1] = '\0';
			
			raw_pkt[3] = (raw_pkt[0]>>4)&0x0F;
			if(ParseVANPacket(addr, raw_pkt+3, raw_len+1, ptr)) //valid packet
				ptr->deliverRAWPacketToPlugins(addr, pkt, 2*raw_len + 8);

			i=0;
			raw_len = 0;
		}
	}
}
