#include <Arduino.h>
#include "Config.h"
#include <QuadDecoder.h>
#include <AnalogEncoder.h>
#include <Bounce2.h>
#include <STM32sleep.h>

#include <SPI.h>
#include "SH1106_SPI.h"

#define USE_FRAME_BUFFER

#ifdef USE_FRAME_BUFFER
SH1106_SPI_FB lcd;
#else
SH1106_SPI lcd;
#endif

Bounce UIBtnDebouncer = Bounce();
Bounce PwrBtnDebouncer = Bounce();

uint32_t lastActivity = 0;
uint32_t lastDisplay = 0;
uint32_t lastPwrBtnUp = 0;
uint32_t displayTime = 0;
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
	lcd.clear(true);
	digitalWrite(PIN_RESET,LOW);
	digitalWrite(PIN_CS, LOW);

	//removes external interrupts
	detachInterrupt(PA10);

	//stops tick timer
	Timer3.pause();

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
	pinMode(OLED_ENABLE,OUTPUT);
	pinMode(USB_PULLUP, OUTPUT);
	pinMode(PWR_LED,OUTPUT);

	digitalWrite(OLED_ENABLE, LOW);
	digitalWrite(USB_PULLUP, HIGH);
	digitalWrite(PWR_LED,LOW);

	//config exteral interrupts
	//attachInterrupt(PA10, indexHandler, FALLING

	//setup display and Serial
	Serial.begin(115200);
	lcd.begin(false,true);

	UIBtnDebouncer.attach(UI_BUTTON);
	UIBtnDebouncer.interval(10);
	PwrBtnDebouncer.attach(PWR_BUTTON);
	PwrBtnDebouncer.interval(10);
	quadDecoder.begin();
	analogEncoder.begin();

	//set up a tick interrupt with a interval of 1 ms.
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
		displayTime = micros() - dispStart;
	}

	UIBtnDebouncer.update();
	PwrBtnDebouncer.update();

	//ui buttton
	if(UIBtnDebouncer.rose()){
		state++;
		lastActivity = millis();
		if(state > 1){
			state = 0;
		}
	}

	//pwr button
	if(PwrBtnDebouncer.fell() && (millis() - lastPwrBtnUp) > POWER_DOWN_MIN_DUR){
		shutdown();
	}

	if(PwrBtnDebouncer.read() == LOW){
		lastPwrBtnUp = millis();
	}

	if((millis()-lastActivity) > TIME_OUT){
		shutdown();
	}
}
