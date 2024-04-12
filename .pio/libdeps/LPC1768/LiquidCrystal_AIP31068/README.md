# LiquidCrystal_AIP31068

A library for AiP31068 based I2C/SPI LCD displays

Simple substitute for LiquidCrystal and LiquidCrystal_I2C to speak with LCD displays based on AiP31068 controller.

## Author

Author: Andriy Golovnya
Based on work of great people who wrote LiquidCrystal and LiquidCrystal_I2C.

## Progress

 - Works: I2C access in 8-bit mode (PSB pin set HIGH)
 - Works: SPI access in 8-bit mode (PSB pin set LOW)
 - Not supported by the AiP31068 chip: 4-bit mode

## Dependencies

This library requires [SoftSPIB](https://github.com/red-scorp/SoftSPIB) - Software SPI library with 9-bit transfer support.

## Additional information

[PDF](http://www.newhavendisplay.com/appnotes/datasheets/LCDs/AiP31068.pdf): AiP31068 40 SEG / 16 COM Driver & Controller for Dot Matrix LCD/2 or 3 Line Serial Data Interface
