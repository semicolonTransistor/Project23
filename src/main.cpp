#include <Arduino.h>
#include <QuadDecoder.h>
#include <AnalogEncoder.h>
#include <Blinker.h>
#include <Bounce2.h>
#include <STM32sleep.h>

#include <SPI.h>
#include "hardware.h"
#include "config.h"
#include <U8g2lib.h>
#include "CLI.h"

#define USE_FRAME_BUFFER

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RESET);

Bounce UIBtn = Bounce();
Bounce PwrBtn = Bounce();
Blinker PwrLed = Blinker();

uint32_t lastActivity = 0;
uint32_t lastDisplay = 0;
uint32_t lastPwrBtnUp = 0;
uint32_t lastSensorRead = 0;
uint8_t state = 0;

void indexHandler(){
	quadDecoder.reset();
}

void tick(){
	quadDecoder.processDecoder();
	analogEncoder.processAnalogEncoder();

}

//shut everything down and prepare for sleep.
void shutdown(){
	//clears and shuts down the oled

	//removes external interrupts
	detachInterrupt(PA10);

	//shuts down the rest of the stm32
	adc_disable_all();
	setGPIOModeToAllPins(GPIO_INPUT_ANALOG);
	disableAllPeripheralClocks();

	//enable WKUP pin,
	rcc_clk_enable(RCC_PWR);
	PWR_BASE->CSR |= PWR_CSR_EWUP;

	//sweet dreams
	goToSleep(STANDBY);
}

void setup(void)	{

	//config pins
	PWR_BASE->CSR &= (~PWR_CSR_EWUP); //disables WKUP pin and returns PA0 to GPIO
	pinMode(UI_BUTTON,INPUT_PULLDOWN);
	pinMode(PWR_BUTTON, INPUT_PULLDOWN);
	pinMode(INDEX,INPUT_PULLUP);
	pinMode(BATT_STAT, INPUT_PULLUP);
	pinMode(R_DIV_ENABLE, OUTPUT);

	pinMode(PWR_ENABLE,OUTPUT);
	pinMode(PWR_LED,OUTPUT);

	pinMode(BATT_MON, INPUT_ANALOG);
	pinMode(VBUS_MON, INPUT_ANALOG);

	digitalWrite(PWR_ENABLE, HIGH);
	digitalWrite(R_DIV_ENABLE, LOW);

	//config exteral interrupts
	//attachInterrupt(PA10, indexHandler, FALLING

	//setup display and Serial
	Serial.begin(115200);
	display.begin();
	display.setBusClock(40000000UL);
	CLI_Init();

	UIBtn.attach(UI_BUTTON);
	UIBtn.interval(10);
	PwrBtn.attach(PWR_BUTTON);
	PwrBtn.interval(10);
	PwrLed.attach(PWR_LED);
	PwrLed.interval(1000);
	PwrLed.set(BlinkerMode::On);

	quadDecoder.begin();
	analogEncoder.begin();


	//set up a tick interrupt with a interval of 1 ms.
	Timer2.setPeriod(1000);
	Timer2.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	Timer2.setCompare1(1);
	Timer2.attachCompare1Interrupt(tick);
	Timer2.refresh();
	Timer2.resume();

}

void loop(void){
	if(millis()-lastDisplay >= 20){
		lastDisplay = millis();
		switch(state){
			case 255:
				display.clearBuffer();
				display.setFont(u8g2_font_ncenB14_tf);
				display.drawStr(0,15,"Goldfish");
				display.drawStr(0,32,"Electronrics");
				display.setFont(u8g2_font_6x10_tf);
    		display.drawStr(0,55,"Project 23");
				display.drawStr(0,63,"Firmware: v0.0.0");
				break;
			case 0:
				display.clearBuffer();
				display.setFont(u8g2_font_timR10_tr);
				display.setCursor(0, 11);
				display.print("Quadrature Encoder:");
				display.setCursor(0,25);
				display.print("Pos: ");
				display.print(quadDecoder.getCount());
				display.setCursor(0,39);
				display.print("Vel: ");
				display.print(quadDecoder.getVelocity());
				break;
			case 1:
				display.clearBuffer();
				display.setFont(u8g2_font_timR10_tr);
				display.setCursor(0, 11);
				display.print("Analog Encoder:");
				display.setCursor(0,25);
				display.print("Pos: ");
				display.print(analogEncoder.getCount());
				display.setCursor(0,39);
				display.print("Vel: ");
				display.print(analogEncoder.getVelocity());
				break;
		}
		display.sendBuffer();
		if(CLI_Stream){
			Serial.print(quadDecoder.getCount());
			Serial.print(",");
			Serial.print(quadDecoder.getVelocity());
			Serial.print(",");
			Serial.print(analogEncoder.getCount());
			Serial.print(",");
			Serial.print(quadDecoder.getVelocity());
			Serial.println();
			lastActivity = millis();
		}
	}

	UIBtn.update();
	PwrBtn.update();
	PwrLed.update();

	//ui buttton
	if(UIBtn.rose()){
		state++;
		lastActivity = millis();
		if(state > 1){
			state = 0;
		}
	}

	//pwr button
	if(PwrBtn.fell()){
		if((millis() - lastPwrBtnUp) > POWER_DOWN_MIN_DUR){
			shutdown();
		}else{
			switch(state){
				case 0:
				case 1:
				quadDecoder.reset();
				break;
				case 2:
				analogEncoder.reset();

			}
		}
	}

	if(millis() - lastSensorRead >= 1){
		lastSensorRead = millis();
		//tick();
	}

	if(PwrBtn.read() == LOW){
		lastPwrBtnUp = millis();
	}

	if(digitalRead(BATT_STAT) == LOW){
		PwrLed.set(BlinkerMode::Blinking);
	}else{
		PwrLed.set(BlinkerMode::On);
	}

	if((millis()-lastActivity) > TIME_OUT){
		shutdown();
	}

	CLI_Update();
	delay(2);

}
