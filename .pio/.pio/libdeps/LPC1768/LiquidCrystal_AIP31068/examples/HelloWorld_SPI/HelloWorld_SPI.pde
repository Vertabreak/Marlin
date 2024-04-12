//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.0
#include <LiquidCrystal_AIP31068_SPI.h>

LiquidCrystal_AIP31068_SPI lcd(2,20,4);  // set the LCD cs pin to 3 for a 20 chars and 4 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  lcd.setCursor(2,1);
  lcd.print("Ywrobot Arduino!");
   lcd.setCursor(0,2);
  lcd.print("Arduino LCM IIC 2004");
   lcd.setCursor(2,3);
  lcd.print("Power By Ec-yuan!");
}


void loop()
{
}
