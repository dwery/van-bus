;	Author:    	Graham Auld <vanbus@graham.auld.me.uk>
;	Code:		Code for 18F2620 to decode VAN bus packets and send them out the UART at 500kB
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.


;Revision history
;3.1	first public source release
;	this still is my current working copy, it still includes debug pins to allow
;	monitoring of the decoding stage and sample points
;	carefull removing these as the current timing takes these into account for accurate decodes

;


	LIST P=18F2620				;directive to define processor
	#include P18F2620.INC			;processor specific variable definitions


	#define	BAUDNUM		.4		; baud  500000 ;need short link at 5v for this one to work happily

	#define	LF		0x0a		;<ENTER> character 

	#define Decoded		PORTC,2		;set if data is ready to be sent out
	#define VANInPin	PORTB,1,0	;Event Ticker
	#define	RAK		Flags,0,0

;******************************************************************************
		CBLOCK	0x000	;Register Bank 0 / Access RAM Area.
		LoopCount1
		LoopCounter2
		Temp2
		Flags
		Counter
		endc

;********************************************************************
;	Macro 'Char2Pc' sends text Char to pc 
;---------------------------------------------------------------------

Char2Pc macro Character
				movlw	Character
				W2Pc
				endm
;********************************************************************

;********************************************************************
;	Macro 'W2Pc' sends wreg to pc 
;	created 3rd april 2007
;---------------------------------------------------------------------

W2Pc macro
				btfss	TXSTA,TRMT		;check if tx shuft reg is free
				goto 	$-2			;not ready
				movwf	TXREG			;send char
				endm
;********************************************************************

;I expect a 10MHz external crystal
;Configuration bits
	CONFIG	OSC = HSPLL  
	CONFIG	FCMEN = OFF
	CONFIG	IESO = OFF
	CONFIG	PWRT = ON
	CONFIG	BOREN = OFF
	CONFIG	BORV = 0
	CONFIG	WDT = OFF
	CONFIG	WDTPS = 1
	CONFIG	MCLRE = ON
	CONFIG	LPT1OSC = OFF

	CONFIG	CCP2MX = PORTBE
	CONFIG	STVREN = ON
	CONFIG	LVP = OFF

	CONFIG	XINST = OFF
	CONFIG	DEBUG = OFF
	CONFIG	CP0 = OFF
	CONFIG	CP1 = OFF
	CONFIG	CP2 = OFF
	CONFIG	CPB = OFF
	CONFIG	WRT0 = OFF
	CONFIG	WRT1 = OFF
	CONFIG	WRT2 = OFF
	CONFIG	WRTB = OFF
	CONFIG	WRTC = OFF
	CONFIG	EBTR0 = OFF
	CONFIG	EBTR1 = OFF
	CONFIG	EBTR2 = OFF

	CONFIG	EBTRB = OFF
;

;*********************************************************************************
;Reset vector
	ORG	0x0000
	goto	Main
;*********************************************************************************
;High priority interrupt vector
	ORG	0x0008
HiPriInt
	retfie	FAST
;*********************************************************************************
;Start of main program

Main
;init serial port


	bsf	TRISC,7								;rx
	bcf	TRISC,6								;tx

	movf 	RCREG,W                         ; flush receive buffer
	movf 	RCREG, W
	
	movlw 	0x90                             ; set up receive options
	movwf 	RCSTA
	movlw 	0x26                             ; set up transmit options
	movwf 	TXSTA
	
	clrf 	SPBRGH
	bcf 	BAUDCON,BRG16
	movlw	BAUDNUM
	movwf 	SPBRG
	

	clrf	T2CON	;set prescalers 1:1
	movlw	.255 ;.120
	movwf	PR2	;set timer2 period register

	movlw	0x0F
	movwf	ADCON1	;disable adc


	bcf		TRISB,4
	bsf		TRISC,5
	bsf		TRISC,4
	bsf		TRISB,1
	bsf		TRISB,0	;(pullup on txd input of mcp2551)


	
	bcf		TRISC,0
	bcf		TRISC,1
	bcf		TRISC,2
	bcf		TRISC,3
	bcf		TRISB,3
	bcf		TRISB,5
	

life	
start
	clrf	Counter
	bcf		PORTB,3
	bcf		PORTB,4
	bcf		PORTB,5
	bcf		Decoded
	btfsc	VANInPin	
	goto	$-2			;wait for low
	call	delay1TS
	btfsc	VANInPin	;low for 1TS
	goto	start
	call	delay1TS
	btfsc	VANInPin	;low for 2TS
	goto	start	
	call	delay1TS
	btfsc	VANInPin	;low for 3TS
	goto	start

	btfss	VANInPin	;wait for going ihgh
	goto	$-2
	call	delay1TS
	btfss	VANInPin	;high for 1ts
	goto	start
	call	delay1TS
	btfss	VANInPin	;high for 2ts
	goto	start
	call	delay1TS
	btfss	VANInPin	;high for 3ts
	goto	start

;found preamble
	bsf		PORTB,3
	
;so look for start bit
	btfsc	VANInPin	
	goto	$-2			;wait for low
	btfss	VANInPin	
	goto	$-2			;wait for high
	call delay1TS
	
	bsf		PORTB,5
;decoded start bit, ready for packt decode
	


rxloop
	lfsr	0,0x100
	;delay 1/2TS
	call	delay05TS
	clrf	INDF0	;default zeros
	
	;sample bit,3
	bsf		PORTC,0
	btfsc	VANInPin
	bsf		INDF0,3	
	bcf		PORTC,0
	
	;delay 1
	call delay1TS	
	
	;sample bit,2
	bsf		PORTC,0
	btfsc	VANInPin
	bsf		INDF0,2
	bcf		PORTC,0
	
	;delay 1	
	call delay1TS
	
	;sample bit,1
	bsf		PORTC,0
	btfsc	VANInPin
	bsf		INDF0,1
	bcf		PORTC,0
	
	;delay 1
	call delay1TS
	
	;sample bit,0
	bsf		PORTC,0
	btfsc	VANInPin
	bsf		INDF0,0
	bcf		PORTC,0

	movf	INDF0,w
	call	Nib2PcFast	;4th sample has been done - output nibble on serial port

	;start timer
	;and wait for an edge...
	clrf	TMR2
	bsf		T2CON,TMR2ON	;start timer...
	
	btfss	INDF0,0
	goto	waitforhigh
	;if bit0 is low wait for high
	btfsc	VANInPin
	goto	$-2
	goto	nextbit
	
waitforhigh
	;if bit0 is high, wait for low
	btfss	VANInPin
	goto	$-2

nextbit
	;so the edge has happened
	;stop the timer
	bcf		T2CON,TMR2ON	;stop timer...
	
	
	;samp
	;0us    4us   12us
	;xxxxxxx/xxxxxxxx	manchester (not EOD)
	;xxxxxxxxxxxxxxx/   violated (EOD)
	;_______/------
	;0      40    120
	
	
	;so - if TMR2 < 80 then there was an edge quick so manchester
	;if TMR2 > 80 then the edge was late and EOD
	movlw	.80
	cpfslt	TMR2	;skip if F<W
	goto	found_EOD ;so we make it here if the edge was late representing EOD!
	;if not then we're looking for more data still
	;but we still need to allow the time slice to occur, so give it 3/4 timeslice ish...
	call	delay075TS
	goto	rxloop
	
;;;;;;;;;;;;;;;;;;;


found_EOD
	;this is signaled by 
	bsf	PORTB,4

	call	delay1TS
	call	delay05TS
	movlw	'N'
	btfss	VANInPin
	movlw	'A'
	W2Pc
		
	Char2Pc	LF	;*nix is LF (\n)
	goto	start	;look for another preamble+start bit (new frame)



delay1TS
	movlw	.24
	movwf	LoopCount1
theloop
	decfsz	LoopCount1,f
	goto	theloop
	return

delay05TS
	movlw	.10
	movwf	LoopCount1
theloop1
	decfsz	LoopCount1,f
	goto	theloop1
	return

delay075TS
	movlw	.19
	movwf	LoopCount1
theloop12
	decfsz	LoopCount1,f
	goto	theloop12
	nop
	nop
	nop
	nop
	nop
	return



Nib2PcFast
	movwf	Temp2
	sublw	0x09		;test >9
	bnn		NibSmallFast
	movlw	0x07		;offset to get from 9 to A in Ascii
	addwf	Temp2,f
NibSmallFast
	movlw	0x30		;shift the value to 0-9 range
	addwf	Temp2,w
	
	movwf	TXREG
	return


Nib2Pc
	movwf	Temp2
	sublw	0x09		;test >9
	bnn		NibSmall
	movlw	0x07		;offset to get from 9 to A in Ascii
	addwf	Temp2,f
NibSmall
	movlw	0x30		;shift the value to 0-9 range
	addwf	Temp2,w
	btfss	TXSTA,TRMT					;check if tx shuft reg is free
	goto 	$-2	
	movwf	TXREG
	return




	END



