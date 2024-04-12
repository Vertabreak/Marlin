#include <SPI.h>
#include <SoftSPIB.h>

// Create a new SPI port with:
// Pin 2 = MOSI,
// Pin 3 = MISO,
// Pin 4 = SCK
SoftSPIB mySPI(12, 13, 4);

void setup() {
	mySPI.begin();
	mySPI.setBitOrder(MSBFIRST);
	Serial.begin(9600);
}

void loop() {
	static uint16_t v = 0x0123;

	Serial.print("Sending value: ");
	Serial.print(v, HEX);
	uint16_t in = mySPI.transferBits(v, 12);
	Serial.print(" Got value: ");
	Serial.print(in, HEX);
	Serial.println(v == in ? " PASS" : " FAIL");
	delay(1000);
	v++;
	v &= 0x03FF;
}