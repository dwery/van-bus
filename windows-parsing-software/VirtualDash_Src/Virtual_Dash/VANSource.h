#pragma once
#include "../common/inc/plugins/plugins_defs.h"

// Virtual class that represent a VAN data source.
// The real source can be UART, file etc.
//
class CVANSource
{
public:
	CVANSource(void);
	virtual ~CVANSource(void);

	// return 1 if error,
	virtual uint8 OpenSource(char* src, char* arg, uint32 options);
	virtual void CloseSource(void);

	// This function bloc until we have a byte
	virtual uint8 ReadByte();

	// This function cancel a blocking read
	virtual void AbortRead();

	//this function write a buffer
	virtual void WriteData(uint8* data, uint16 len);
};
