#include <iostream>
#include <wiringPi.h>
#include "ST7565/ST7565.h"

using namespace std;

const char activeSymbol[] PROGMEM = {
	0b00000000,
	0b00000000,
	0b00011000,
	0b00100100,
	0b01000010,
	0b01000010,
	0b00100100,
	0b00011000};

const char inactiveSymbol[] PROGMEM = {
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000};

// pin 11 - LCD reset (RST)
// pin 23 - Serial clock out (SCLK)
// pin 13 - Data/Command select (RS or A0)
// pin 19 - Serial data out (SID)
// pin 05 - LCD chip select (CS)
ST7565 *lcd = new ST7565(11, 23, 13, 19, 24);

void drawLines()
{
	for (int16_t i = 0; i < DISPLAY_WIDTH; i += 4)
	{
		lcd->drawLine(0, 0, i, DISPLAY_HEIGHT - 1);
		lcd->display();
		delay(10);
	}
	for (int16_t i = 0; i < DISPLAY_HEIGHT; i += 4)
	{
		lcd->drawLine(0, 0, DISPLAY_WIDTH - 1, i);
		lcd->display();
		delay(10);
	}
	delay(250);

	lcd->clear();
	for (int16_t i = 0; i < DISPLAY_WIDTH; i += 4)
	{
		lcd->drawLine(0, DISPLAY_HEIGHT - 1, i, 0);
		lcd->display();
		delay(10);
	}
	for (int16_t i = DISPLAY_HEIGHT - 1; i >= 0; i -= 4)
	{
		lcd->drawLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1, i);
		lcd->display();
		delay(10);
	}
	delay(250);

	lcd->clear();
	for (int16_t i = DISPLAY_WIDTH - 1; i >= 0; i -= 4)
	{
		lcd->drawLine(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, i, 0);
		lcd->display();
		delay(10);
	}
	for (int16_t i = DISPLAY_HEIGHT - 1; i >= 0; i -= 4)
	{
		lcd->drawLine(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, 0, i);
		lcd->display();
		delay(10);
	}
	delay(250);
	lcd->clear();
	for (int16_t i = 0; i < DISPLAY_HEIGHT; i += 4)
	{
		lcd->drawLine(DISPLAY_WIDTH - 1, 0, 0, i);
		lcd->display();
		delay(10);
	}
	for (int16_t i = 0; i < DISPLAY_WIDTH; i += 4)
	{
		lcd->drawLine(DISPLAY_WIDTH - 1, 0, i, DISPLAY_HEIGHT - 1);
		lcd->display();
		delay(10);
	}
	delay(250);
}

void drawRect(void)
{
	for (int16_t i = 0; i < DISPLAY_HEIGHT / 2; i += 2)
	{
		lcd->drawRect(i, i, DISPLAY_WIDTH - 2 * i, DISPLAY_HEIGHT - 2 * i);
		lcd->display();
		delay(10);
	}
}

void fillRect(void)
{
	uint8_t color = 1;
	for (int16_t i = 0; i < DISPLAY_HEIGHT / 2; i += 3)
	{
		lcd->setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
		lcd->fillRect(i, i, DISPLAY_WIDTH - i * 2, DISPLAY_HEIGHT - i * 2);
		lcd->display();
		delay(10);
		color++;
	}
}

void drawCircle(void)
{
	for (int16_t i = 0; i < DISPLAY_HEIGHT; i += 2)
	{
		lcd->clear();
		lcd->drawCircle(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, i);
		lcd->display();
		delay(100);
	}
	delay(1000);
	lcd->clear();

	// This will draw the part of the circel in quadrant 1
	// Quadrants are numberd like this:
	//   0010 | 0001
	//  ------|-----
	//   0100 | 1000
	//
	lcd->drawCircleQuads(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 4, 0b00000001);
	lcd->display();
	delay(200);
	lcd->drawCircleQuads(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 4, 0b00000011);
	lcd->display();
	delay(200);
	lcd->drawCircleQuads(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 4, 0b00000111);
	lcd->display();
	delay(200);
	lcd->drawCircleQuads(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 4, 0b00001111);
	lcd->display();
}

int main()
{
	lcd->config(22, 0, 0, 0, 4);
	lcd->clear();
	lcd->display();
	lcd->displayOn();

	lcd->setTextAlignment(TEXT_ALIGN_LEFT);
    lcd->setFont(ArialMT_Plain_10);
    lcd->drawString(0, 0, "Hello world");
    lcd->setFont(ArialMT_Plain_16);
    lcd->drawString(0, 10, "Hello world");
    lcd->setFont(ArialMT_Plain_24);
    lcd->drawString(0, 26, "Hello world");
	lcd->display();

	// lcd->flipScreenVertically();

	// lcd->setContrast(20);

	// drawCircle();
	// delay(1000);
	// lcd->clear();

	// drawLines();
	// delay(1000);
	// lcd->clear();

	// drawRect();
	// delay(1000);
	// lcd->clear();

	// fillRect();
	// delay(1000);
	// lcd->clear();
}