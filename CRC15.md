The CRC algorithm used on the VAN bus is a standard CRC whose parameters are:

|Name| 	CRC-15 VAN ISO/11519–3|
|:---|:-----------------------|
|Width| 	15                    |
|Poly| 	[F9D](F9D.md)         |
|Init| 	7FFF                  |
|RefIn| 	False                 |
|RefOut| 	False                 |
|XorOut| 	7FFF                  |
|Check| 	6B39                  |

See http://www.repairfaq.org/filipg/LINK/F_crc_v34.html#CRCV_005 for an explanation of the above mentioned parameters.