#ifndef __ST7565__
#define __ST7565__
#include <cstdint>
#include <iostream>
#include <cstring>

// NanoPi Neo WiringPi header
#include <wiringPi.h>
#include <wiringShift.h>

// Fonts
#include "DisplayFonts.h"

using namespace std;

// Adafruit ST7565 | PCB S68063-1 | HomePhone VNPT
// 1. GND	ground
// 2. RST	Reset
// 3. SCLK	Serial clock
// 4. A0	sometiimes called RS
// 5. K	LED anode +
// 6. A	LED cathode -
// 7. SID	Serial Input Data
// 8. CS	Chip select	GND
// 9. VCC/VDD	3.3V power
// 10. GND	ground
// SPI 4 PIN RST, SCLK, A0, SID, [CS -> GND]

// Display settings
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH_MAX 127
#define DISPLAY_HEIGHT_MAX 63
#define DISPLAY_BUFFER_SIZE 1024
#define ENABLE_PARTIAL_UPDATE true
#define ST7565_STARTBYTES 0 // Some LCD set 0, Adafruit LCD set 1

enum DISPLAY_COLOR
{
  BLACK = 0,
  WHITE = 1,
  INVERSE = 2
};

enum DISPLAY_TEXT_ALIGNMENT
{
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_RIGHT = 1,
  TEXT_ALIGN_CENTER = 2,
  TEXT_ALIGN_CENTER_BOTH = 3
};

// LCD commands from datasheet
#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF
#define CMD_SET_DISP_START_LINE 0x40
#define CMD_SET_PAGE 0xB0
#define CMD_SET_COLUMN_UPPER 0x10
#define CMD_SET_COLUMN_LOWER 0x00
#define CMD_SET_ADC_NORMAL 0xA0
#define CMD_SET_ADC_REVERSE 0xA1
#define CMD_NONE_ALL_PIXEL 0xA4
#define CMD_ALL_PIXEL 0xA5
#define CMD_SET_DISP_NORMAL 0xA6
#define CMD_SET_DISP_REVERSE 0xA7
#define CMD_SET_COM_NORMAL 0xC8
#define CMD_SET_COM_REVERSE 0xC0
#define CMD_SET_ALLPTS_NORMAL 0xA4
#define CMD_SET_ALLPTS_ON 0xA5
#define CMD_SET_BIAS_9 0xA2
#define CMD_SET_BIAS_7 0xA3
#define CMD_RMW 0xE0
#define CMD_RMW_CLEAR 0xEE
#define CMD_INTERNAL_RESET 0xE2
#define CMD_SET_POWER_CONTROL 0x28
#define CMD_SET_RESISTOR_RATIO 0x20
#define CMD_SET_VOLUME_FIRST 0x81
#define CMD_SET_VOLUME_SECOND 0
#define CMD_SET_STATIC_OFF 0xAC
#define CMD_SET_STATIC_ON 0xAD
#define CMD_SET_STATIC_REG 0x0
#define CMD_SET_BOOSTER_FIRST 0xF8
#define CMD_SET_BOOSTER_234 0
#define CMD_SET_BOOSTER_5 1
#define CMD_SET_BOOSTER_6 3
#define CMD_NOP 0xE3
#define CMD_TEST 0xF0

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
  {                         \
    int16_t t = a;          \
    a = b;                  \
    b = t;                  \
  }
#endif

/**
 * Thư viện giao tiếp màn hình G-Phone/Homephone VNPT
 * Chuẩn hóa theo thư viện của @squix78 bởi @tienthanh2509
 */
class ST7565
{
protected:
  uint8_t sid = 0, sclk = 0, cs = 0;
  uint8_t a0 = 0, rst = 0;

  DISPLAY_TEXT_ALIGNMENT textAlignment = TEXT_ALIGN_LEFT;
  DISPLAY_COLOR color = BLACK;

  const char *fontData = ArialMT_Plain_10;

#ifdef ENABLE_PARTIAL_UPDATE
  int16_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
#endif

  // the memory buffer for the LCD
  uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);

  void init(void);
  void sendInitCommands(void);
  void sendCommand(uint8_t c);
  void sendData(uint8_t c);
  void spiWrite(uint8_t c);

  void updateBoundingBox(int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax);

public:
  ST7565(uint8_t rst, uint8_t sclk, uint8_t a0, uint8_t sid, uint8_t cs);

  void config(uint8_t contrast, bool negative, bool rotation, bool mirror, uint8_t resistor_ratio);

  // Write the buffer to the display memory
  void display(void);

  // Clear the local pixel buffer
  void clear(void);

  // This doesnt touch the buffer, just clears the display RAM - might be handy
  void clearDisplay(void);

  /* Display functions */

  // Turn the display on
  void displayOn(void);

  // Turn the display offs
  void displayOff(void);

  // Inverted display mode
  void invertDisplay(void);

  // Normal display mode
  void normalDisplay(void);

  // Set display contrast
  void setContrast(uint8_t contrast);

  // Turn the display upside down
  void flipScreenVertically();

  /* Drawing functions */
  // Sets the color of all pixel operations
  void setColor(DISPLAY_COLOR color);

  // Draw a pixel at given position
  void setPixel(int16_t x, int16_t y);
  // Get a pixel at given position
  uint8_t getPixel(int16_t x, int16_t y);

  // Draw a line from position 0 to position 1
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

  // Draw the border of a rectangle at the given location
  void drawRect(int16_t x, int16_t y, int16_t width, int16_t height);

  // Fill the rectangle
  void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);

  // Draw the border of a circle
  void drawCircle(int16_t x, int16_t y, int16_t radius);

  // Draw all Quadrants specified in the quads bit mask
  void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads);

  // Fill circle
  void fillCircle(int16_t x, int16_t y, int16_t radius);

  // Draw a line horizontally
  void drawHorizontalLine(int16_t x, int16_t y, int16_t length);

  // Draw a lin vertically
  void drawVerticalLine(int16_t x, int16_t y, int16_t length);

  // Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
  // a unsigned byte value between 0 and 100
  void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);

  // Draw a bitmap in the internal image format
  void drawFastImage(int16_t x, int16_t y, int16_t width, int16_t height, const char *image);

  // Draw a XBM
  void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, const char *xbm);
};

#endif
