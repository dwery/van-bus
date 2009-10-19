#include "StdAfx.h"
#include "vansource.h"

CVANSource::CVANSource(void)
{
}

CVANSource::~CVANSource(void)
{
}


uint8 CVANSource::ReadByte(void)
{
	OutputDebugString("ERROR : CVANSource::ReadByte Should no be called!\n");
	Sleep(1000);
	return 0;
}

void CVANSource::AbortRead(void)
{
	OutputDebugString("ERROR : CVANSource::AbortByte Should no be called!\n");
}

void CVANSource::WriteData(uint8* data, uint16 len)
{
	OutputDebugString("ERROR : CVANSource::WriteData Should no be called!\n");
	Sleep(1000);
}

uint8 CVANSource::OpenSource(char* src, char* arg, uint32 options)
{
	OutputDebugString("ERROR : CVANSource::OpenSource Should no be called!\n");
	return 1;
}

void CVANSource::CloseSource(void)
{
	OutputDebugString("ERROR : CVANSource::CloseSource Should no be called!\n");
}