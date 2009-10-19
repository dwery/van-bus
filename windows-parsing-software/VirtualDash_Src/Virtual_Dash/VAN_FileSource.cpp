#include "StdAfx.h"
#include "van_filesource.h"

CVAN_FileSource::CVAN_FileSource(void)
{
	f=NULL;
	m_filter = 0;
}

CVAN_FileSource::~CVAN_FileSource(void)
{
	CloseSource();
}

uint8 CVAN_FileSource::OpenSource(char* src, char* arg, uint32 options)
{
	OutputDebugString("CVAN_FileSource::OpenSource : opening file \n");
	OutputDebugString(src);
	CloseSource();

	f = fopen(src, "rb");
	if(f)
	{
		m_filter = options;
		return 0;
	}

	OutputDebugString("ERROR : cannot open file\n");

	return 1;
}

void CVAN_FileSource::CloseSource(void)
{
	if(f)
	{
		fclose(f);
		f = NULL;
	}
}

uint8 CVAN_FileSource::_ReadByte(void)
{
	uint8 c = 0;
	
	while(1)
	{
		if(fread(&c,sizeof(uint8), 1, f))
		{
			break;
		}
		else
		{
			OutputDebugString("End of file... Going to begining.\n");
			fseek(f, 0, SEEK_SET );
			if(fread(&c, sizeof(uint8), 1, f))
			{
				break;
			}
			else
			{
				OutputDebugString("Error : Empty log file?\n");
				c=0;
				break;
			}
		}
	}
	
	return c;
}

uint8 CVAN_FileSource::ReadByte(void)
{
	uint8 c;
	uint8 val;
	
	if(f == NULL)
	{
		Sleep(100);
		return 0;
	}

	while(1)
	{
		c = _ReadByte();
		if(m_filter == VAN_FILE_ASCII_ONLY)
		{
			if(c != 0x0a)
				break;	
			else
				Sleep(20); // just to slow down
		}
		else if(m_filter == VAN_FILE_ASCII_TO_HEX)
		{
			if((c >= 0x30) && (c <= 0x39))
			{
				c = c - 0x30;
			}
			else if((c >= 'A') && (c <= 'F'))
			{
				c = c - 'A' + 0x0a;
			}
			else if((c >= 'a') && (c <= 'a'))
			{
				c = c - 'a' + 0x0a;
			}
			else
				continue;

			c = c<<4;
			while(1)
			{
				val = _ReadByte();

				if((val >= 0x30) && (val <= 0x39))
				{
					c |= val - 0x30;
					break;
				}
				else if((val >= 'A') && (val <= 'F'))
				{
					c |= val - 'A' + 0x0a;
					break;
				}
				else if((val >= 'a') && (val <= 'a'))
				{
					c |= val - 'a' + 0x0a;
					break;
				}
				
			}
			break;
		}
		else
			break;
	}

	//Sleep(1); // just to slow down
	return c;
}

void CVAN_FileSource::AbortRead(void)
{
}

void CVAN_FileSource::WriteData(uint8* data, uint16 len)
{
	OutputDebugString("ERROR : CVAN_FileSource::WriteData Should no be called!\n");
	Sleep(1000);
}

