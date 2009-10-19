#include "StdAfx.h"
#include "van_uartsource.h"
#include "serial_win32.h"

CVAN_UartSource::CVAN_UartSource(void)
{
	
}

CVAN_UartSource::~CVAN_UartSource(void)
{
	CloseSource();
}

uint8 CVAN_UartSource::OpenSource(char* src, char* arg, uint32 options)
{
	return Ser32_Init(src, arg, options);
}

void CVAN_UartSource::CloseSource(void)
{
	Ser32_AbortRead();
	Ser32_Close();
}

uint8 CVAN_UartSource::ReadByte(void)
{
	
	return Ser32_readByte();
}

void CVAN_UartSource::AbortRead(void)
{
	Ser32_AbortRead();
}

void CVAN_UartSource::WriteData(uint8* data, uint16 len)
{
	
	Ser32_sendbytes(data, len);
}

