// Txt_VAN_Parser.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "stdio.h"

#include "../../common/inc/plugins/plugins_defs.h"
#include "../../common/inc/plugins/parser_plugins_defs.h"

#include "../common/van_parser.h"


CParserPlugin* ptr = NULL;
uint8 pkt[255];
uint8 raw_pkt[156];
uint16 raw_len;
uint16 i;
int addr;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


__declspec(dllexport) void _stdcall Init_Parser(CParserPlugin* callbacks)
{
	OutputDebugString("Txt_VAN_Parser : Init_Parser\n");
	i = 0;
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

	if(c == 0x0d) // end of packet?
	{
		pkt[i] = 0;
		OutputDebugString((char*)pkt);
		OutputDebugString("\n");

		if(i&1)
		{
			OutputDebugString("ERROR : The packet length is not even\n");
		}
		else
		{
			if(i < 10)
			{
				OutputDebugString("Invalid van packet len (<10!)\n");
			}
			else
			{
				// check that there is a valid header
				// it should contain the addr (3 bytes in hexa + one space)
				if(!isHex(pkt[0]) || !isHex(pkt[1]) || !isHex(pkt[2]) || (pkt[3] != ' '))
				{
					OutputDebugString("Invalid van packet : no addr found\n");
				}
				else
				{

					if(sscanf((char*)pkt,"%03x", &addr) != 1)
					{
						OutputDebugString("ERROR : Cannot decode addr...\n");
					}
					else
					{
						raw_pkt[0] = 0x08;
						if(pkt[4] == 'R')
							raw_pkt[0] |= 0x02;
						if(pkt[5] == 'A')
							raw_pkt[0] |= 0x04;
						if(pkt[6] == 'T')
							raw_pkt[0] |= 0x01;
						
						for(raw_len = 1; raw_len<((i/2)-4); ++raw_len)
							raw_pkt[raw_len] = GetHex(pkt + (2*raw_len + 6));
						if(pkt[i-2] == 'A')
							raw_pkt[raw_len] = 0xEF;
						else
							raw_pkt[raw_len] = 0xEE;
						++raw_len;

						if(ParseVANPacket(addr, raw_pkt, raw_len, ptr)) //valid packet
							ptr->deliverRAWPacketToPlugins(addr, pkt, i);
					}
				}
			}
		}
		

		i=0;
	}
	else
	{
		pkt[i] = c;
		++i;
	}
}
