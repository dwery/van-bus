#pragma once
#include "vansource.h"
#include <stdio.h>

#include "serial_win32.h"

class CVAN_UartSource :
	public CVANSource
{

private:

public:
	CVAN_UartSource(void);
	virtual ~CVAN_UartSource(void);

	// return 1 if error,
	virtual uint8 OpenSource(char* src, char* arg, uint32 options);
	virtual void CloseSource(void);

	// This function bloc until we have a byte
	virtual uint8 ReadByte();

	virtual void AbortRead();

	//this function write a buffer
	virtual void WriteData(uint8* data, uint16 len);
};
