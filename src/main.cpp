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

int main()
{
	lcd->config(22, 0, 0, 0, 4);
	lcd->clear();
	lcd->display();
	lcd->displayOn();

	// lcd->setColor(BLACK);
	// for (int i = 0; i < 64; i++)
	// {
	// 	for (int j = 0; j < 128; j++)
	// 	{
	// 		lcd->setPixel(j, i);
	// 		lcd->display();
	// 		delay(1);
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

	lcd->setColor(BLACK);
	for (int j = 0; j < 64; j++)
	{
		lcd->drawHorizontalLine(30, j, 30);
		// lcd->drawHorizontalLine(0, 7, 128);
		// lcd->drawHorizontalLine(0, 14, 128);
		// lcd->drawHorizontalLine(0, 8, 128);
		// lcd->drawHorizontalLine(0, 63, 128);
		lcd->display();
		delay(10);
	}
}