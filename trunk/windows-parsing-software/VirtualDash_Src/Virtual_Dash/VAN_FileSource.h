#pragma once
#include "vansource.h"
#include <stdio.h>

#define VAN_FILE_ASCII_ONLY		0 // only filter 0x0a
#define VAN_FILE_ASCII_TO_HEX	1 // The file is a text file containing hexa values.
#define VAN_FILE_HEX			2 // the file is a binary file,no filtering

class CVAN_FileSource :
	public CVANSource
{

private:
	FILE* f;
	uint32 m_filter;

	uint8 _ReadByte(void);

public:
	CVAN_FileSource(void);
	virtual ~CVAN_FileSource(void);

	// return 1 if error,
	virtual uint8 OpenSource(char* src, char* arg, uint32 options);
	virtual void CloseSource(void);

	// This function bloc until we have a byte
	virtual uint8 ReadByte();

	virtual void AbortRead();
	
	//this function write a buffer
	virtual void WriteData(uint8* data, uint16 len);
};
