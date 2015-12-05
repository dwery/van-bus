# The Auldy interface for VAN monitoring #

## Introduction ##

Having looked at the speed of the comfort VAN I was attempting to monitor I decided it should be possible to capture all packets on the bus without the need for filtering and buffer filling as with the RT3 interface.

To this end I started from scratch.



## Details ##

My Requirements:

  * Have every entire VAN frame made avaliable on my PC for logging and decoding
  * Preferably use a ASCII format that can be piped straight into log files


I looked at VAN controllers but found them incredibly difficult to get hold of. This left me one course of action - bit banging.


The code in SVN auldy\_interface/VAN-USB/pic\_code/ has been released at version 3.1. It should run happily on any 18F series PIC with a 10MHz crystal. Most timings are done using simple software delays with the exception of the use of the Timer 2 module for detecting the end of data condition.


The code works like this:

  1. Looks for VAN frame preamble
  1. When it finds preamble it sets PORTB bit 3
  1. Looks for VAN fram start bit
  1. When it finds the start bit it sets PORTB bit 5
  1. Now the loop starts reading 4 bits (5 time slices at a time)
    * After bits 3-0 have been sampled, an ascii nibble is transmitted through the UART and 500kbps
    * No Enhanced Manchester Code violation causes the loop to repeat
    * Enhanced Manchester Code violation signals the End Of Data and breaks the loop
  1. t the appropriate time, the Acknowledge bit is sampled
    * ACK causes 'A' to be transmitted via UART
    * No ACK causes 'N' to be transmitted via UART
  1. `*`nix style LF (\n) is transmitted via UART to delimit VAN frames (no time for CR+LF if there are back to back VAN frames on the wire!)


The practical upshot of the strangely high UART baud rate is that a nibble can be transmitted to the PC before the next nibble arrives on the VAN bus allowing for continuous reception pass through.



My code for controlling a Pioneer head unit simply stored bytes until an End Of Data was signalled then processed them looking for the 9C4 IDEN and then checking the set bits in order to drive an output to the head unit. (this is why the code you see stores the incoming data via an indirect register address)



Improvements I can see would be using timers for the sampling, checking the CRC in time to send an ACK (that could be tight for time without an interrupt based receiver)


Transmitting frame acknowledges would be a start but the really exciting bit would be to add VAN frame transmission and replys to read request messages.


Emulating a CDC or Sat-Nav unit would be ace!