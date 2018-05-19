#include <Arduino.h>
#include <QuadDecoder.h>

#include <SPI.h>
#include "SH1106_SPI.h"

#define USE_FRAME_BUFFER

#ifdef USE_FRAME_BUFFER
SH1106_SPI_FB lcd;
#else
SH1106_SPI lcd;
#endif

uint32_t lastDisplay = 0;
uint32_t lastUpdate = 0;
uint32_t time = 0;

void tick(){
	quadDecoder.processDecoder();
}

void setup(void)
{
	Serial.begin(115200);
	lcd.begin(false,true);
	quadDecoder.begin();
	Timer3.setPeriod(1000);
	Timer3.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	Timer3.setCompare1(1);
	Timer3.attachCompare1Interrupt(tick);
	Timer3.refresh();
	Timer3.resume();
}

void loop(void)
{
	if(millis()-lastDisplay >= 20){
		uint32_t dispStart = micros();
		lcd.clear(false);
		lastDisplay = millis();
		int len = 0;
		len += lcd.print("Encoder:");
		lcd.renderString(0,0,len);
		len = 0;
		len += lcd.print(F("Position:"));
		len += lcd.print(quadDecoder.getCount());
		lcd.renderString(0,1,len);
		len = 0;
		len += lcd.print(F("Velocity:"));
		len += lcd.print(quadDecoder.getVelocity());
		lcd.renderString(0,2,len);
		lcd.renderAll();
		time = micros() - dispStart;
	}
/*	size_t len;
	unsigned long time = micros();
	lcd.clear();
	time = micros() - time;
	lcd.print(F("The time it clear the screen is: "));
	lcd.print(time);
	Serial.print(F("The time it clear the screen is: "));
	Serial.println(time);
#ifdef USE_FRAME_BUFFER
	lcd.renderAll();
#endif
	delay(3000);

	lcd.clear();
	time = micros();
#ifdef USE_FRAME_BUFFER
	len = lcd.print(F("!\"#$%'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz !\"#$%'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrs"));
	lcd.renderString(0, 0, len);
#else
	len = lcd.print(F("!\"#$%'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz !\"#$%'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopq"));
#endif
	time = micros() - time;
	delay(3000);

	lcd.clear();
	lcd.print(F("The time it took to print "));
	lcd.print(len);
	lcd.print(F(" chars is: "));
	lcd.print(time);
	Serial.print(F("The time it took to print "));
	Serial.print(len);
	Serial.print(F(" chars is: "));
	Serial.println(time);
#ifdef USE_FRAME_BUFFER
	lcd.renderAll();
#endif
	delay(3000);

	lcd.clear();
	lcd.gotoXY(5,3);
	lcd.print(F("Test gotoXY"));
#ifdef USE_FRAME_BUFFER
	lcd.renderAll();
#endif
	delay(3000);

	lcd.clear();
	time = micros();
	lcd.writeBitmap(bmp, 10, 2, 25, 3);
#ifdef USE_FRAME_BUFFER
	lcd.renderAll();
#endif
	time = micros() - time;
	delay(3000);

	lcd.clear();
	lcd.print(F("The time it took to draw a 25x3 (25x18) bitmap is: "));
	lcd.print(time);
	Serial.print(F("The time it took to draw a 25x3 (25x18) bitmap is: "));
	Serial.println(time);
#ifdef USE_FRAME_BUFFER
	lcd.renderAll();
#endif
	delay(3000);

#ifdef USE_FRAME_BUFFER
	lcd.clear();
	time = micros();
	lcd.writeRect(5, 5, 50, 40);
	lcd.writeLine(75, 3, 75, 35);
	lcd.writeLine(60, 10, 60, 40);
	lcd.writeLine(10, 47, 60, 47);
	lcd.renderAll();
	time = micros() - time;
	delay(3000);

	lcd.clear();
	len = lcd.print(F("The time it took draw a rect and 3 lines: "));
	len += lcd.print(time);
	Serial.print(F("The time it took draw a rect and 3 lines: "));
	Serial.println(time);
	lcd.renderString(0, 0, len);
	delay(3000);

	lcd.clear();
	time = micros();
	for (uint8_t row = 0; row < SH1106_Y_PIXELS; row++)
	{
		for (uint8_t col = 0; col < SH1106_X_PIXELS; col++)
		{
			uint8_t pixel = (col + row) % 2;
			lcd.setPixel(col, row, pixel);
		}
	}
	lcd.renderAll();
	time = micros() - time;
	delay(5000);

	lcd.clear();
	lcd.print(F("The time it took to run setPixel on all 8192 pixels and render it: "));
	lcd.print(time);
	Serial.print(F("The time it took to run setPixel on all 8192 pixels and render it: "));
	Serial.println(time);
	lcd.renderAll();
	delay(5000);
#endif*/
}
