# SoftSPIB

Software SPI (Bit) class for Arduino with support of not 8-bit aligned transfers.

## Reason

Some HW components use non 8-bit aligned transfers to work properly.
Unfortunately Arduino HW SPI library do not support this kind of data transfer.
Though, we have to enable it.

## Examples
- Loopback is an example of 12-bit data transfer.

## Author

Maintener: [Andriy Golovnya](http://github.com/red-scorp/).
Based on code of [Majenko Technologies](http://github.com/MajenkoLibraries/SoftSPI).
