#include "SH1106_SPI.h"
#include <SPI.h>

SH1106_SPI::SH1106_SPI()
{
}

void SH1106_SPI::begin(bool invert, bool fastSpi, uint8_t contrast, uint8_t Vpp)
{
	pinMode(PIN_CS, OUTPUT);
	pinMode(PIN_DC,	OUTPUT);
	pinMode(PIN_RESET, OUTPUT);
	SPI.begin();
	if (fastSpi){
		SPI.setClockDivider(SPI_BAUD_PCLK_DIV_2);
	}else{
		SPI.setClockDivider(SPI_BAUD_PCLK_DIV_8);
	}
	//lcd init section

	uint8_t invertSetting = invert ? 0xA7 : 0xA6;
	Vpp &= 0x03;

	// Must reset LCD first!
	digitalWrite(PIN_RESET,LOW);
	delay(1);
	digitalWrite(PIN_RESET,HIGH);


	this->writeLcd(SH1106_COMMAND, 0xAE);    /*display off*/
	this->writeLcd(SH1106_COMMAND, 0x02);    /*set lower column address*/
	this->writeLcd(SH1106_COMMAND, 0x10);    /*set higher column address*/
	this->writeLcd(SH1106_COMMAND, 0x40);    /*set display start line*/
	this->writeLcd(SH1106_COMMAND, 0xB0);    /*set page address*/
	this->writeLcd(SH1106_COMMAND, 0x81);    /*contract control*/
	this->writeLcd(SH1106_COMMAND, contrast);    /*128*/
	this->writeLcd(SH1106_COMMAND, 0xA1);    /*set segment remap*/
	this->writeLcd(SH1106_COMMAND, invertSetting);    /*normal / reverse*/
	this->writeLcd(SH1106_COMMAND, 0xA8);    /*multiplex ratio*/
	this->writeLcd(SH1106_COMMAND, 0x3F);    /*duty = 1/32*/
	this->writeLcd(SH1106_COMMAND, 0xAD);    /*set charge pump enable*/
	this->writeLcd(SH1106_COMMAND, 0x8B);     /*external VCC   */
	this->writeLcd(SH1106_COMMAND, 0x30 | Vpp);    /*0X30---0X33  set VPP   9V liangdu!!!!*/
	this->writeLcd(SH1106_COMMAND, 0xC8);    /*Com scan direction*/
	this->writeLcd(SH1106_COMMAND, 0xD3);    /*set display offset*/
	this->writeLcd(SH1106_COMMAND, 0x00);   /*   0x20  */
	this->writeLcd(SH1106_COMMAND, 0xD5);    /*set osc division*/
	this->writeLcd(SH1106_COMMAND, 0x80);
	this->writeLcd(SH1106_COMMAND, 0xD9);    /*set pre-charge period*/
	this->writeLcd(SH1106_COMMAND, 0x1F);    /*0x22*/
	this->writeLcd(SH1106_COMMAND, 0xDA);    /*set COM pins*/
	this->writeLcd(SH1106_COMMAND, 0x12);
	this->writeLcd(SH1106_COMMAND, 0xDB);    /*set vcomh*/
	this->writeLcd(SH1106_COMMAND, 0x40);
	this->writeLcd(SH1106_COMMAND, 0xAF);    /*display ON*/

	this->clear();
}

size_t SH1106_SPI::write(uint8_t data)
{
	// Non-ASCII characters are not supported.
	if (data < 0x20 || data > 0x7F) return 0;
	if (this->m_Column >= 123)
		this->advanceXY(SH1106_X_PIXELS - this->m_Column);

	uint8_t buf[6];
	memcpy_P(buf, ASCII[data - 0x20], 5);
	buf[5] = 0x00;
	this->writeLcd(SH1106_DATA, buf, 6);
	this->advanceXY(6);
	return 1;
}

void SH1106_SPI::clear()
{
	for (uint8_t j = SH1106_ROWS; j > 0; j--)
	{
		this->gotoXY(0, j-1);
		for (uint8_t i = SH1106_X_PIXELS; i > 0; i--)
			this->writeLcd(SH1106_DATA, 0x00);
	}
	this->gotoXY(0, 0);
}

uint8_t SH1106_SPI::gotoXY(uint8_t x, uint8_t y)
{
	if (x >= SH1106_X_PIXELS || y >= SH1106_ROWS) return SH1106_ERROR;
	this->m_Column = x;
	this->m_Line = y;
    x = x + 2;										// Panel is 128 pixels wide, controller RAM has space for 132,
													// it's centered so add an offset to ram address.

    this->writeLcd(SH1106_COMMAND, 0xB0 + y);		// Set row
    this->writeLcd(SH1106_COMMAND, x & 0xF);		// Set lower column address
    this->writeLcd(SH1106_COMMAND, 0x10 | (x >> 4));// Set higher column address
	return SH1106_SUCCESS;
}

uint8_t SH1106_SPI::writeBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
	if (this->gotoXY(x, y) == SH1106_ERROR) return SH1106_ERROR;
	const uint8_t *maxY = bitmap + height * width;

	for (const uint8_t *y = bitmap; y < maxY; y += width)
	{
		this->writeLcd(SH1106_DATA, y , width);
		this->gotoXY(this->m_Column, this->m_Line + 1);
	}

	this->advanceXY(width);
	return SH1106_SUCCESS;
}

void SH1106_SPI::advanceXY(uint8_t columns)
{
	this->m_Column += columns;
	if (this->m_Column >= SH1106_X_PIXELS)
	{
		this->m_Column %= SH1106_X_PIXELS;
		this->m_Line++;
		this->m_Line %= SH1106_ROWS;
		this->gotoXY(this->m_Column, this->m_Line);
	}
}

void SH1106_SPI::writeLcd(uint8_t dataOrCommand, const uint8_t *data, uint16_t count)
{
	digitalWrite(PIN_CS,LOW);
	if(dataOrCommand == SH1106_COMMAND){
		digitalWrite(PIN_DC,LOW);
	}else{
		digitalWrite(PIN_DC, HIGH);
	}

    for (uint16_t i = count; i > 0; i--)
	{
		SPI.transfer(data[count-i]);
		//SPDR = *data++;
		//while (!(SPSR & _BV(SPIF)));
	}
	digitalWrite(PIN_CS, HIGH);
}

void SH1106_SPI::writeLcd(uint8_t dataOrCommand, uint8_t data)
{
	digitalWrite(PIN_CS,LOW);
	if(dataOrCommand == SH1106_COMMAND){
		digitalWrite(PIN_DC,LOW);
	}else{
		digitalWrite(PIN_DC, HIGH);
	}

	SPI.transfer(data);
	//SPDR = data;
	//while (!(SPSR & _BV(SPIF)));
	digitalWrite(PIN_CS, HIGH);
}
