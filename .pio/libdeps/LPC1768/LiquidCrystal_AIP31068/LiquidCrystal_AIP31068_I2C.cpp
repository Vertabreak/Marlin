// Based on the work by DFRobot

#include "LiquidCrystal_AIP31068_I2C.h"
#include <inttypes.h>
#include <Wire.h>

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#define printIIC(args)	Wire.write(args)
inline size_t LiquidCrystal_AIP31068_I2C::write(uint8_t value) {
	send(value, 1);
	return 1;
}

#else
#include "WProgram.h"

#define printIIC(args)	Wire.send(args)
inline void LiquidCrystal_AIP31068_I2C::write(uint8_t value) {
	send(value, 1);
}

#endif

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal_AIP31068_I2C::LiquidCrystal_AIP31068_I2C(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) {
	_Addr = lcd_Addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
}

void LiquidCrystal_AIP31068_I2C::oled_init() {
	_oled = true;
	init_priv();
}

void LiquidCrystal_AIP31068_I2C::init() {
	init_priv();
}

void LiquidCrystal_AIP31068_I2C::init_priv() {
	Wire.begin();
	_displayfunction = LCD_1LINE | LCD_5x8DOTS | LCD_8BITMODE;
	begin(_cols, _rows);  
}

void LiquidCrystal_AIP31068_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
	delayMicroseconds(50000); 

	// this is according to the hitachi HD44780 datasheet
	// page 45 figure 23

	// Send function set command sequence
	command(LCD_FUNCTIONSET | _displayfunction);
	delayMicroseconds(4500);  // wait more than 4.1ms

	// second try
	command(LCD_FUNCTIONSET | _displayfunction);
	delayMicroseconds(150);

	// third go
	command(LCD_FUNCTIONSET | _displayfunction);

	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	
	home();
  
}

/********** high level commands, for the user! */
void LiquidCrystal_AIP31068_I2C::clear() {
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
	if (_oled) setCursor(0,0);
}

void LiquidCrystal_AIP31068_I2C::home() {
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal_AIP31068_I2C::setCursor(uint8_t col, uint8_t row) {
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal_AIP31068_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_AIP31068_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal_AIP31068_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_AIP31068_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal_AIP31068_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_AIP31068_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal_AIP31068_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal_AIP31068_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal_AIP31068_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal_AIP31068_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal_AIP31068_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal_AIP31068_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_AIP31068_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

//createChar with PROGMEM input
void LiquidCrystal_AIP31068_I2C::createChar(uint8_t location, const char *charmap) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
	    	write(pgm_read_byte_near(charmap++));
	}
}


/*********** mid level commands, for sending data/cmds */

void LiquidCrystal_AIP31068_I2C::command(uint8_t value) {
	send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void LiquidCrystal_AIP31068_I2C::send(uint8_t value, uint8_t mode) {
	uint16_t rs = mode != 0? Rs << 8: 0;
	write8bits(rs | value); 
}

void LiquidCrystal_AIP31068_I2C::write4bits(uint16_t value) {
	controllerWrite((value & 0xFF00) | (value & 0x00FF) << 4);
}

void LiquidCrystal_AIP31068_I2C::write8bits(uint16_t value) {
	controllerWrite(value);
}

void LiquidCrystal_AIP31068_I2C::controllerWrite(uint16_t _data){
	Wire.beginTransmission(_Addr);
	printIIC((_data >> 8) & 0xFF);
	printIIC((_data >> 0) & 0xFF);
	Wire.endTransmission();   
}

// Alias functions

void LiquidCrystal_AIP31068_I2C::cursor_on(){
	cursor();
}

void LiquidCrystal_AIP31068_I2C::cursor_off(){
	noCursor();
}

void LiquidCrystal_AIP31068_I2C::blink_on(){
	blink();
}

void LiquidCrystal_AIP31068_I2C::blink_off(){
	noBlink();
}

void LiquidCrystal_AIP31068_I2C::load_custom_character(uint8_t char_num, uint8_t *rows){
		createChar(char_num, rows);
}

void LiquidCrystal_AIP31068_I2C::printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}


// unsupported API functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void LiquidCrystal_AIP31068_I2C::off(){}
void LiquidCrystal_AIP31068_I2C::on(){}
void LiquidCrystal_AIP31068_I2C::setDelay (int cmdDelay,int charDelay) {}
uint8_t LiquidCrystal_AIP31068_I2C::status(){return 0;}
uint8_t LiquidCrystal_AIP31068_I2C::keypad (){return 0;}
uint8_t LiquidCrystal_AIP31068_I2C::init_bargraph(uint8_t graphtype){return 0;}
void LiquidCrystal_AIP31068_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void LiquidCrystal_AIP31068_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}
void LiquidCrystal_AIP31068_I2C::setContrast(uint8_t new_val){}
#pragma GCC diagnostic pop
	
