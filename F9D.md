The polynomial for the VAN CRC is

> x<sup>15 + x</sup>11 + x<sup>10 + x</sup>9 + x<sup>8 + x</sup>7 + x<sup>4 + x</sup>3 + x^2

To translate it to a hexadecimal poly value, you will need to take a 15 bit value and write a one in each interesting bit position.

The 15 bits are numbered from 14 to 0 . The leftmost (MSB) bit represents x<sup>14 (x</sup>15 is implicit) and the rightmost x^0 (which must always be set in any CRC poly, so it’s often not mentioned in the formula).

The result is:

> 000111110011101

or

> 0xF9D