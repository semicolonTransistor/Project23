#include <Arduino.h>
#include <QuadDecoder.h>
#include <AnalogEncoder.h>
#include <SupplyReader.h>
#include <Blinker.h>
#include <Bounce2.h>
#include <STM32sleep.h>

#include <SPI.h>
#include "hardware.h"
#include "config.h"
#include "bitmaps.h"
#include <U8g2lib.h>
#include "CLI.h"

#define USE_FRAME_BUFFER

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RESET);

Bounce UIBtn = Bounce();
Bounce PwrBtn = Bounce();
Bounce RevLimit = Bounce();
Bounce FwdLimit = Bounce();

Blinker pwrLed = Blinker();
Blinker fwdLed = Blinker();
Blinker revLed = Blinker();

uint32_t lastActivity = 0;
uint32_t lastDisplay = 0;
uint32_t lastPwrBtnUp = 0;
uint8_t state = 255;
uint8_t battState = 0;

volatile uint16_t battVoltsRaw = 0;
volatile uint16_t vbusVoltsRaw = 0;
volatile uint16_t vccRaw = 0;

void indexHandler(){
	quadDecoder.reset();
}

void tick(){
	supplyReader.processSupply();
	vccRaw = supplyReader.getRawReading();
	quadDecoder.processDecoder();
	analogEncoder.processAnalogEncoder();
	battVoltsRaw = analogRead(BATT_MON);
	vbusVoltsRaw = analogRead(VBUS_MON);
}

//shut everything down and prepare for sleep.
void shutdown(){
	//shuts down power to other circuits
	digitalWrite(PWR_ENABLE, LOW);

	//disables all interrupts
	noInterrupts();

	//removes external interrupts
	detachInterrupt(PA10);

	//stops timers
	timer_disable_all();

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
	pinMode(BATT_STAT, INPUT_PULLUP);

	pinMode(INDEX,INPUT_PULLUP);
	pinMode(FWD_LIMIT, INPUT);
	pinMode(REV_LIMIT, INPUT);

	pinMode(PWR_ENABLE, OUTPUT);
	pinMode(R_DIV_ENABLE, OUTPUT);
	pinMode(PWR_LED, OUTPUT);
	pinMode(FWD_LED, OUTPUT);
	pinMode(REV_LED, OUTPUT);

	pinMode(BATT_MON, INPUT_ANALOG);
	pinMode(VBUS_MON, INPUT_ANALOG);

	digitalWrite(PWR_ENABLE, HIGH);
	digitalWrite(R_DIV_ENABLE, LOW);

	//setup display and Serial
	Serial.begin(115200);
	display.begin();
	display.setBusClock(40000000UL);
	CLI_Init();

	UIBtn.attach(UI_BUTTON);
	UIBtn.interval(10);

	PwrBtn.attach(PWR_BUTTON);
	PwrBtn.interval(10);

	FwdLimit.attach(FWD_LIMIT);
	FwdLimit.interval(10);

	RevLimit.attach(REV_LIMIT);
	RevLimit.attach(10);

	pwrLed.attach(PWR_LED);
	pwrLed.interval(1000);
	pwrLed.set(BlinkerMode::On);

	fwdLed.attach(FWD_LED);
	fwdLed.interval(1000);

	revLed.attach(REV_LED);
	revLed.interval(1000);

	quadDecoder.begin();
	analogEncoder.begin();
	supplyReader.begin();

	//set up a tick interrupt with a interval of 1 ms.
	timer_init(TIMER3);
	timer_set_prescaler(TIMER3, 1);
	timer_set_reload(TIMER3, 36000);
	timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, tick);
	timer_generate_update(TIMER3);
	timer_resume(TIMER3);


}

void loop(void){
	if(millis()-lastDisplay >= 20){
		lastDisplay = millis();
		display.clearBuffer();

		//Draw battery indicator
		noInterrupts();
		uint16_t battVoltsRawCp = battVoltsRaw;
		uint16_t vbusVoltsRawCp = vbusVoltsRaw;
		uint16_t vccRawCp = vccRaw;
		interrupts();
		if(battVoltsRawCp >= 2480){
			battState = 3;
		}

		if((battState < 2 && battVoltsRawCp >= 2428) || (battState > 2 && battVoltsRawCp <= 2460) ){
				battState = 2;
		}

		if((battState < 1 && battVoltsRawCp >= 2376) || (battState > 1 && battVoltsRawCp <= 2408) ){
				battState = 1;
		}

		if(battVoltsRawCp <= 2356){
			battState = 0;
		}

		if(vbusVoltsRawCp <= 2855){
			switch (battState) {
				case 1:
				display.drawXBM(0, 0, Batt1_width, Batt1_height, Batt1_bits);
				break;

				case 2:
				display.drawXBM(0, 0, Batt2_width, Batt2_height, Batt2_bits);
				break;

				case 3:
				display.drawXBM(0, 0, Batt3_width, Batt3_height, Batt3_bits);
				break;

				default:
				display.drawXBM(0, 0, Batt0_width, Batt0_height, Batt0_bits);
			}
		}else{
			display.drawXBM(0, 0, BattUSB_width, BattUSB_height, BattUSB_bits);
		}

		display.setFont(u8g2_font_ncenB08_tf);
		display.setCursor(20, 9);
		display.print(1.2 * (4096.0/(float)vccRawCp));
		Serial.print(1.2 * (4096.0/(float)vccRawCp));
		Serial.print(",");
		Serial.println(vccRawCp);

		switch(state){
			case 255:
				display.setFont(u8g2_font_ncenB10_tf);
				display.drawStr(0,24,"Goldfish");
				display.drawStr(0,38,"Electronrics");
				display.setFont(u8g2_font_6x10_tf);
    		display.drawStr(0,48,"Project 23");
				display.drawStr(0,58,"Firmware: v0.7.0");
				break;
			case 0:
				display.setFont(u8g2_font_timR10_tr);
				display.setCursor(0, 22);
				display.print("Quadrature Encoder:");
				display.setCursor(0,36);
				display.print("Pos: ");
				display.print(quadDecoder.getCount());
				display.setCursor(0,50);
				display.print("Vel: ");
				display.print(quadDecoder.getVelocity());
				break;
			case 1:
				display.setFont(u8g2_font_timR10_tr);
				display.setCursor(0, 22);
				display.print("Analog Encoder:");
				display.setCursor(0,36);
				display.print("Pos: ");
				display.print(analogEncoder.getCount());
				display.setCursor(0,50);
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
	FwdLimit.update();
	RevLimit.update();

	pwrLed.update();
	fwdLed.update();
	revLed.update();

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

	if(PwrBtn.read() == LOW){
		lastPwrBtnUp = millis();
	}

	if(digitalRead(BATT_STAT) == LOW){
		pwrLed.set(BlinkerMode::Blinking);
	}else{
		pwrLed.set(BlinkerMode::On);
	}

	if((millis()-lastActivity) > TIME_OUT){
		shutdown();
	}

	CLI_Update();
	delay(2);

}
