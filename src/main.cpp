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

//ask U8g2 to use a full frame buffer for the display since we have plenty of ram.
#define USE_FRAME_BUFFER

//Display instantiation
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RESET);

//Debounced inputs instantiation
Bounce UIBtn = Bounce();
Bounce PwrBtn = Bounce();
Bounce RevLimit = Bounce();
Bounce FwdLimit = Bounce();

//Blink capabale LEDs instantiation
Blinker pwrLed = Blinker();
Blinker fwdLed = Blinker();
Blinker revLed = Blinker();

//variables for tracking time
uint32_t lastActivity = 0;
uint32_t lastDisplay = 0;
uint32_t lastPwrBtnUp = 0;

//display state machine.
uint8_t state = 255;
uint8_t battState = 0;

//volatiles for passing data from interrupts to main code
volatile uint16_t battVoltsRaw = 0;
volatile uint16_t vbusVoltsRaw = 0;
volatile uint16_t vccRaw = 0;

//interrupt handlerler for encoder index
void indexHandler(){
	quadDecoder.reset();
}

//Timed interrupt for processing sensor inputs, 1kHz
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

	PWR_BASE->CSR &= (~PWR_CSR_EWUP); 				//disables WKUP pin and returns PA0 to GPIO

	//Buttons
	pinMode(UI_BUTTON,INPUT_PULLDOWN);				//sets up the UI button as input with pulldown resistor
	pinMode(PWR_BUTTON, INPUT_PULLDOWN);			//sets up the power button as input with pulldown resistor

	//Sensorport inputs
	pinMode(INDEX,INPUT_PULLUP);
	pinMode(FWD_LIMIT, INPUT);
	pinMode(REV_LIMIT, INPUT);

	//Battery charing stat input setup
	pinMode(BATT_STAT, INPUT_PULLUP);					//sets up the battery charge stat input as input pullup

	//Internal control outputs setup
	pinMode(PWR_ENABLE, OUTPUT);
	pinMode(R_DIV_ENABLE, OUTPUT);

	//LEDs pin setup
	pinMode(PWR_LED, OUTPUT);
	pinMode(FWD_LED, OUTPUT);
	pinMode(REV_LED, OUTPUT);

	//Battery and USB Vbus monitoring setup
	pinMode(BATT_MON, INPUT_ANALOG);
	pinMode(VBUS_MON, INPUT_ANALOG);

	digitalWrite(PWR_ENABLE, HIGH);
	digitalWrite(R_DIV_ENABLE, LOW);

	//Serial init
	Serial.begin(USB_SERIAL_BAUDRATE);
	CLI_Init();

	//display init
	display.begin();
	display.setBusClock(32000000UL); //set clockspeed to 32MHz


//Debounced inputs init
	UIBtn.attach(UI_BUTTON);
	UIBtn.interval(10);

	PwrBtn.attach(PWR_BUTTON);
	PwrBtn.interval(10);

	FwdLimit.attach(FWD_LIMIT);
	FwdLimit.interval(10);

	RevLimit.attach(REV_LIMIT);
	RevLimit.interval(10);

	//Blink capable outputs init
	pwrLed.attach(PWR_LED);
	pwrLed.interval(1000);
	pwrLed.set(BlinkerMode::On);

	fwdLed.attach(FWD_LED);
	fwdLed.interval(1000);

	revLed.attach(REV_LED);
	revLed.interval(1000);

	//sensor processor init
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

		Serial.println(vccRawCp);

		switch(state){
			case 255:
				display.drawXBM(0, 14, goldfish_logo_width, goldfish_logo_height, goldfish_logo_bits);
				display.setFont(u8g2_font_ncenB08_tf);
				display.drawStr(34,24,"Goldfish");
				display.drawStr(34,38,"Electronrics");
				display.setFont(u8g2_font_6x10_tf);
    		display.drawStr(0,48,"Project 23");
				display.drawStr(0,58,"Firmware: v0.7.0");
				break;
			case 254:
				display.setFont(u8g2_font_6x10_tf);
				display.drawStr(0,24,"Error!");
				display.drawStr(0,34,"Battery Criticaly LOW");
	    	display.drawStr(0,44,"Please Charge");
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

		if(millis() >= INTRO_SCREEN_DURATION && state == 255){
				state = 0;
		}

		if(vccRawCp >= 1536 && state != 255){
			state = 254;
		}

		if(state == 254 && vccRawCp <= 1512 ){
			state = 0;
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
		lastActivity = millis();
		if(state < 128){
			state++;
		}
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
	delay(1);

}
