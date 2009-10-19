#include "stdafx.h"
#include "serial_win32.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>


HANDLE COM_handle = INVALID_HANDLE_VALUE;
HANDLE hComReadThread = INVALID_HANDLE_VALUE;
HANDLE COM_hSemaphore = INVALID_HANDLE_VALUE;

#define COM_RX_MAX_BUFF	2048

CRITICAL_SECTION	COM_rxMutex;
CRITICAL_SECTION	COM_txMutex;


uint8 COM_rxbuf[COM_RX_MAX_BUFF];
uint16 COM_rxStartInd = 0;
uint16 COM_rxStopInd = 0;
uint16 COM_rxSize = 0;

uint8 abort_rx = 0;
/*****************************************************************************/

void Ser32_AbortRead(void)
{
	if(COM_hSemaphore == INVALID_HANDLE_VALUE)
		return;

	OutputDebugString("Ser32_AbortRead...\n");
	abort_rx = 1;

	if(!ReleaseSemaphore( COM_hSemaphore, 1, NULL) )
		printf("AbortRead : ReleaseSemaphore error!\n");
	
}

uint8 Ser32_readByte(void)
{
	uint8 c;

	if((COM_handle == INVALID_HANDLE_VALUE) || (abort_rx == 1))
	{
		Sleep(100);
		return 0;
	}
	// Try to enter the semaphore gate.
	WaitForSingleObject( COM_hSemaphore, INFINITE);

	if(abort_rx)
	{
		//abort_rx = 0;
		return 0;
	}

	EnterCriticalSection(&COM_rxMutex);

	c = COM_rxbuf[COM_rxStartInd];

	COM_rxSize = COM_rxSize - 1;

	LeaveCriticalSection(&COM_rxMutex);
	
	COM_rxStartInd = COM_rxStartInd + 1;
	if(COM_rxStartInd >= COM_RX_MAX_BUFF)
		COM_rxStartInd = COM_rxStartInd - COM_RX_MAX_BUFF;
	
	return c;
}

uint16 Ser32_read(uint8 * buff, uint16 length)
{
	uint16 i;

	i = 0;

	while(1)
	{
		if(i == length)
			break;

		if(buff)
			buff[i] = Ser32_readByte();
		else
			Ser32_readByte(); // just discard the byte...
		++i;
	}
	
	return 0;
}



void COM_HandleASuccessfulRead(uint8* rxData, uint16 dwRead)
{
	if(!dwRead)
		return;

	EnterCriticalSection(&COM_rxMutex);


	COM_rxSize = COM_rxSize + dwRead;
	if(COM_rxSize > COM_RX_MAX_BUFF)
	{
		printf("Error! : COM_RxSize > RX_MAX_BUFF!\n");
		printf("***** STOP *****\n");
		while(1)
			Sleep(1000);
	}

	if((COM_rxStopInd + dwRead) == COM_RX_MAX_BUFF)
	{
		COM_rxStopInd = 0;
	}
	else if((COM_rxStopInd + dwRead) < COM_RX_MAX_BUFF)
	{
		COM_rxStopInd = COM_rxStopInd + dwRead;
	}
	else
		printf("***HandleASuccessfulRead:: FATAL ERROR...\n");

	
	if(!ReleaseSemaphore( COM_hSemaphore, dwRead, NULL) )
		printf("HandleASuccessfulRead : ReleaseSemaphore error!\n");
	

	LeaveCriticalSection(&COM_rxMutex);
}


void COM_FlushUart(void)
{
	PurgeComm(COM_handle, PURGE_RXCLEAR);
}

DWORD WINAPI COM_rxThreadFunc(LPVOID lpParameter)
{

	DWORD dwRead;
	BOOL fWaitingOnRead = FALSE;
	OVERLAPPED osReader = {0};
	DWORD dwRes;
	uint16 theRxSize;
	uint16 numberToRead;
	COMSTAT		comStat;
	DWORD		errors;

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	 
	if(osReader.hEvent == NULL)
	{
		// Error creating overlapped event; abort.
		OutputDebugString("error creating overlapped event\n");
	}
	 
	while(1)
	{
		if (!fWaitingOnRead) 
		{
			EnterCriticalSection(&COM_rxMutex);
			theRxSize = COM_rxSize;
			LeaveCriticalSection(&COM_rxMutex);

			if(theRxSize >= COM_RX_MAX_BUFF)
			{
				Sleep(10);
				
				continue;
			}

			if(COM_handle == INVALID_HANDLE_VALUE)
				break;
			/* find out how many to read */
			if(!ClearCommError(COM_handle, &errors, &comStat))
			{
				/*	serious error	*/
				OutputDebugString("********Serious error in read file in rx thread********\n");
				break;
			}
			numberToRead = (uint16)((comStat.cbInQue > 0) ? comStat.cbInQue : 100);
			
			if((theRxSize + numberToRead) > COM_RX_MAX_BUFF)
			{
				numberToRead = COM_RX_MAX_BUFF - theRxSize;
			}

			if((COM_rxStopInd + numberToRead) > COM_RX_MAX_BUFF)
			{
				numberToRead = COM_RX_MAX_BUFF - COM_rxStopInd;
			}
			
			
			// Issue read operation.
			if(!ReadFile(COM_handle, COM_rxbuf + COM_rxStopInd, numberToRead, &dwRead, &osReader)) 
			{
				if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				{
					OutputDebugString("********Error in communications*******\n");
					break;
				}
				else
					fWaitingOnRead = TRUE;
			}
			else 
			{    
				// read completed immediately
				if(dwRead)
					COM_HandleASuccessfulRead(COM_rxbuf + COM_rxStopInd, (uint16)dwRead);
				
							
				
			}
		}

 
		if(fWaitingOnRead) 
		{
			dwRes = WaitForSingleObject(osReader.hEvent, INFINITE);
			switch(dwRes)
			{
				// Read completed.
				case WAIT_OBJECT_0:
					if (!GetOverlappedResult(COM_handle, &osReader, &dwRead, FALSE))
						OutputDebugString("Error in communication\n");
					else
					{
						// Read completed successfully.
						if(dwRead)
							COM_HandleASuccessfulRead(COM_rxbuf + COM_rxStopInd, (uint16)dwRead);
							
						
						
					}
					//  Reset flag so that another operation can be issued.
					fWaitingOnRead = FALSE;
					break;
			 
				case WAIT_TIMEOUT:
					// Operation isn't complete yet. fWaitingOnRead flag isn't
					// changed since I'll loop back around, and I don't want
					// to issue another read until the first one finishes.
					//
					// This is a good time to do some background work.
					break;                       
			 
				default:
					// Error in the WaitForSingleObject; abort.
					// This indicates a problem with the OVERLAPPED structure's
					// event handle.
					OutputDebugString("************** CRITICAL : Error in the WaitForSingleObject *****\n");
					
					OutputDebugString("COM_rxThreadFunc exited\n");
					return 0;
					
					break;
			}
		}

	}

	OutputDebugString("COM_rxThreadFunc exited\n");

	return 0;
}

void Ser32_Close(void)
{
	if(COM_handle == INVALID_HANDLE_VALUE)
		return;
	OutputDebugString("Ser32_Close : Closing thread\n");
	CloseHandle(COM_handle);
	COM_handle = INVALID_HANDLE_VALUE;
	WaitForSingleObject(hComReadThread, 10000);
	CloseHandle(hComReadThread);
	hComReadThread = INVALID_HANDLE_VALUE;

	DeleteCriticalSection(&COM_rxMutex);
	DeleteCriticalSection(&COM_txMutex);

	CloseHandle(COM_hSemaphore);

	OutputDebugString("Ser32_Close : Closed\n");
}
/*****************************************************************************/
// return 1 if error
uint8 Ser32_Init(char *arg, char* param, uint32 options)
{
	DCB	config;
	COMMTIMEOUTS timeouts;

	InitializeCriticalSection(&COM_rxMutex);
	InitializeCriticalSection(&COM_txMutex);

	//ZeroMemory(rxbuf, RX_MAX_BUFF);
	FillMemory(COM_rxbuf, COM_RX_MAX_BUFF, 0xFF);

	COM_rxStartInd = 0;
	COM_rxStopInd = 0;
	COM_rxSize = 0;

	abort_rx = 0;

	OutputDebugString("Opening "); OutputDebugString(arg); OutputDebugString(" ("); OutputDebugString(param); OutputDebugString(")\n");
	// Open the com port required
	COM_handle = CreateFile
	(
	 	arg,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		/*FILE_ATTRIBUTE_NORMAL |*/ FILE_FLAG_OVERLAPPED,
		0
	);

	if (COM_handle == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("**** Error : Cannot open com port\n");
		return 1;
	}

	// make sure everything is normal

	memset(&config, 0, sizeof (config));

	//BuildCommDCB("baud=115200 parity=E data=8 stop=1", &config);
	//BuildCommDCB("baud=115200 parity=N data=8 stop=1", &config);
	BuildCommDCB(param, &config);

	if(options == SER32_HARDWARE_FLOW_CONTROL)
	{
		config.fOutxCtsFlow = 1;

		config.fRtsControl = RTS_CONTROL_HANDSHAKE;
		config.fBinary = TRUE;
		config.fDsrSensitivity = FALSE;
		config.fOutX = FALSE;
		config.fInX = FALSE;
		config.XonLim = 128;
		config.XoffLim = 256;
	}

	// setup the com port state
	if(!SetCommState(COM_handle, &config))
		OutputDebugString("**** Error : cannot set uart parameters!\n");

	// setup the com port timeouts

	timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 300000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;


	SetCommTimeouts (COM_handle, &timeouts);

	SetCommMask(COM_handle, 0);

	// Setup the fifo depth
	if(!SetupComm(COM_handle, 512, 512))
		OutputDebugString("***** Error : SetupComm\n");


	COM_hSemaphore = CreateSemaphore(NULL, 0, 0xFFFF, NULL);

	if(COM_hSemaphore == NULL) 
		OutputDebugString("CreateSemaphore error\n");
	
	PurgeComm(COM_handle, PURGE_RXCLEAR);
	hComReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) COM_rxThreadFunc, NULL, 0, NULL);

	return 0;
}


BOOL COM_WriteABuffer(char * lpBuf, DWORD dwToWrite)
{
   OVERLAPPED osWrite = {0};
   DWORD dwWritten;
   DWORD dwRes;
   BOOL fRes;
 
   // Create this write operation's OVERLAPPED structure's hEvent.
   osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (osWrite.hEvent == NULL)
      // error creating overlapped event handle
      return FALSE;
 
   // Issue write.
   if(!WriteFile(COM_handle, lpBuf, dwToWrite, &dwWritten, &osWrite)) 
   {
      if (GetLastError() != ERROR_IO_PENDING) 
	  { 
         // WriteFile failed, but isn't delayed. Report error and abort.
		  printf("WriteFile failed\n");
          fRes = FALSE;
      }
      else
	  {
         // Write is pending.
         dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
         switch(dwRes)
         {
            // OVERLAPPED structure's event has been signaled. 
            case WAIT_OBJECT_0:
                 if (!GetOverlappedResult(COM_handle, &osWrite, &dwWritten, FALSE))
                       fRes = FALSE;
                 else
                  // Write operation completed successfully.
                  fRes = TRUE;
                 break;
            
            default:
                 // An error has occurred in WaitForSingleObject.
                 // This usually indicates a problem with the
                // OVERLAPPED structure's event handle.
				printf("An error has occurred in WaitForSingleObject\n");
                fRes = FALSE;
                break;
         }
      }
   }
   else
      // WriteFile completed immediately.
      fRes = TRUE;
 
   CloseHandle(osWrite.hEvent);
   return fRes;
}

void Ser32_sendbytes(uint8 *buf, uint16 n)
{
	if(COM_handle != INVALID_HANDLE_VALUE)
		COM_WriteABuffer((char*)buf, n);
}

