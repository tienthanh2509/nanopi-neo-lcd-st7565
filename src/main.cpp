#include <iostream>
#include <wiringPi.h>
#include "ST7565/ST7565.h"

using namespace std;

// pin 11 - LCD reset (RST)
// pin 23 - Serial clock out (SCLK)
// pin 13 - Data/Command select (RS or A0)
// pin 19 - Serial data out (SID)
// pin 05 - LCD chip select (CS)
ST7565 *lcd = new ST7565(11, 23, 13, 19, 24);

// Adapted from Adafruit_SSD1306
void drawLines()
{
	cout<<"drawLines"<<endl;
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

// Adapted from Adafruit_SSD1306
void drawRect(void)
{
	cout<<"drawRect"<<endl;
	for (int16_t i = 0; i < DISPLAY_HEIGHT / 2; i += 2)
	{
		lcd->drawRect(i, i, DISPLAY_WIDTH - 2 * i, DISPLAY_HEIGHT - 2 * i);
		lcd->display();
		delay(10);
	}
}

// Adapted from Adafruit_SSD1306
void fillRect(void)
{
	cout<<"fillRect"<<endl;
	uint8_t color = 1;
	for (int16_t i = 0; i < DISPLAY_HEIGHT / 2; i += 3)
	{
		lcd->setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
		lcd->fillRect(i, i, DISPLAY_WIDTH - i * 2, DISPLAY_HEIGHT - i * 2);
		lcd->display();
		delay(10);
		color++;
	}
	// Reset back to WHITE
	lcd->setColor(WHITE);
}

// Adapted from Adafruit_SSD1306
void drawCircle(void)
{
	cout<<"drawCircle"<<endl;
	for (int16_t i = 0; i < DISPLAY_HEIGHT; i += 2)
	{
		lcd->drawCircle(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, i);
		lcd->display();
		delay(10);
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

void printBuffer(void)
{
	// // Initialize the log buffer
	// // allocate memory to store 8 lines of text and 30 chars per line.
	// lcd->setLogBuffer(5, 30);

	// // Some test data
	// const char *test[] = {
	// 	"Hello",
	// 	"World",
	// 	"----",
	// 	"Show off",
	// 	"how",
	// 	"the log buffer",
	// 	"is",
	// 	"working.",
	// 	"Even",
	// 	"scrolling is",
	// 	"working"};

	// for (uint8_t i = 0; i < 11; i++)
	// {
	// 	lcd->clear();
	// 	// Print to the screen
	// 	lcd->println(test[i]);
	// 	// Draw it to the internal screen buffer
	// 	lcd->drawLogBuffer(0, 0);
	// 	// Display it on the screen
	// 	lcd->display();
	// 	delay(500);
	// }
}

int main()
{
	lcd->config(23, 0, 0, 0, 4);
	lcd->clear();
	lcd->displayOn();

	// lcd->setColor(WHITE);
	// lcd->setPixel(10, 10);
	// cout << (char)lcd->getPixel(10, 10);

	// lcd->setColor(BLACK);
	// for (int i = 0; i < 64; i++)
	// {
	// 	for (int j = 0; j < 128; j++)
	// 	{
	// 		lcd->setPixel(j, i);
	// 	}
	// }
	// lcd->display();

	// delay(1000);

	// lcd->setColor(WHITE);
	// for (int i = 0; i < 64; i += 10)
	// {
	// 	for (int j = 0; j < 128; j += 10)
	// 	{
	// 		lcd->setPixel(j, i);
	// 	}
	// }
	// lcd->display();

	// delay(1000);

	// lcd->setColor(INVERSE);
	// for (int i = 0; i < 64; i++)
	// {
	// 	for (int j = 0; j < 128; j++)
	// 	{
	// 		lcd->setPixel(j, i);
	// 	}
	// }
	// lcd->display();

	// drawLines();
	// delay(1000);
	// lcd->clear();

	// lcd->setColor(INVERSE);
	// drawRect();
	// delay(1000);
	// lcd->clear();

	// fillRect();
	// delay(1000);
	// lcd->clear();

	// drawCircle();
	// delay(1000);
	// lcd->clear();

	// printBuffer();
	// delay(1000);
	// lcd->clear();
}