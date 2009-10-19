#ifndef _SER_WIN32_H_
#define _SER_WIN32_H_


#include "../common/inc/types.h"

#define SER32_NO_FLOW_CONTROL		0
#define SER32_HARDWARE_FLOW_CONTROL	1

void Ser32_AbortRead(void);

uint8 Ser32_Init(char *arg, char* param, uint32 options);
void Ser32_Close(void);

uint8 Ser32_readByte(void);
uint16 Ser32_read(uint8 * buff, uint16 length);

void Ser32_sendbytes(uint8 *buf, uint16 n);

#endif // _SER_WIN32_H_