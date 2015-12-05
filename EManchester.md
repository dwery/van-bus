Standard manchester: http://www.wikipedia.com/wiki/Manchester_encoding

From [Design and validation process of in-vehicle embedded electronic systems](http://hal.archives-ouvertes.fr/docs/00/04/44/96/PDF/Chap40-IncarEmbSys-150604-all.pdf):

‘’In enhanced manchester encoding, a binary sequence is divided into blocks of 4 bits and the first three bits are encoded using NRZ code (whose duration is defined as one Time Slot per bit) while the fourth one is encoded using Manchester code (two Time Slots per bit). It means that 4 bits of data are encoded using 5 Time Slots (TS). Thanks to E-Manchester coding, VAN, unlike CAN, doesn’t need bit stuffing for bit synchronisation. This coding is sometimes denoted by 4B/5B. ‘’