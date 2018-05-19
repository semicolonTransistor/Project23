#include <Arduino.h>
#include <QuadDecoder.h>
#include <AnalogEncoder.h>
#include <Bounce2.h>

#include <SPI.h>
#include "SH1106_SPI.h"

#define USE_FRAME_BUFFER

#ifdef USE_FRAME_BUFFER
SH1106_SPI_FB lcd;
#else
SH1106_SPI lcd;
#endif

Bounce debouncer = Bounce();

uint32_t lastDisplay = 0;
uint32_t lastUpdate = 0;
uint32_t time = 0;
uint8_t state = 0;

void tick(){
	quadDecoder.processDecoder();
	analogEncoder.processAnalogEncoder();
	digitalWrite(PC13,digitalRead(PA15));
}

void setup(void)
{
	Serial.begin(115200);
	lcd.begin(false,true);
	pinMode(PC13,OUTPUT);
	pinMode(PA15,INPUT_PULLUP);

	debouncer.attach(PA15);
	debouncer.interval(10);
	quadDecoder.begin();
	analogEncoder.begin();
	Timer3.setPeriod(1000);
	Timer3.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	Timer3.setCompare1(1);
	Timer3.attachCompare1Interrupt(tick);
	Timer3.refresh();
	Timer3.resume();
}

void loop(void){
	if(millis()-lastDisplay >= 20){
		uint32_t dispStart = micros();
		lcd.clear(false);
		lastDisplay = millis();
		int len = 0;
		switch (state) {
			case 0:
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
			break;
			case 1:
			len += lcd.print("Analog:");
			lcd.renderString(0,0,len);
			len = 0;
			len += lcd.print(F("Position:"));
			len += lcd.print(analogEncoder.getCount());
			lcd.renderString(0,1,len);
			len = 0;
			len += lcd.print(F("Velocity:"));
			len += lcd.print(analogEncoder.getVelocity());
			lcd.renderString(0,2,len);
			break;
		}
		lcd.renderAll();
		time = micros() - dispStart;
	}
	debouncer.update();
	if(debouncer.fallingEdge()){
		state++;
		if(state > 1){
			state = 0;
		}
	}
}
