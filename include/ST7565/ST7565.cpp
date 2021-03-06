#include "ST7565.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// a handy reference to where the pages are on the screen
const uint8_t pagemap[] = {7, 6, 5, 4, 3, 2, 1, 0};
// const uint8_t pagemap[] = {3, 2, 1, 0, 7, 6, 5, 4};

// ------------------------------------------------------------------------------------------------
ST7565::ST7565(uint8_t rst, uint8_t sclk, uint8_t a0, uint8_t sid, uint8_t cs)
{
  this->rst = rst;
  this->sclk = sclk;
  this->a0 = a0;
  this->sid = sid;
  this->cs = cs;

  init();
}
// ------------------------------------------------------------------------------------------------
// BEGIN: Private Method
// ST7565 Core Method
void ST7565::init()
{
  //
  wiringPiSetupPhys();

  // set pin directions
  pinMode(this->rst, OUTPUT);
  pinMode(this->sclk, OUTPUT);
  pinMode(this->a0, OUTPUT);
  pinMode(this->sid, OUTPUT);

  // toggle RST low to reset; CS low so it'll listen to us
  if (this->cs > 0)
  {
    digitalWrite(this->cs, LOW);
    pinMode(this->cs, OUTPUT);
  }

  digitalWrite(this->rst, LOW);
  delay(500);
  digitalWrite(this->rst, HIGH);

  sendInitCommands();
}

void ST7565::sendInitCommands(void)
{
  // LCD bias select
  sendCommand(CMD_SET_BIAS_7);

  // ADC select
  sendCommand(CMD_SET_ADC_NORMAL);
  // SHL select
  sendCommand(CMD_SET_COM_NORMAL);
  // Initial display line
  sendCommand(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  sendCommand(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  delay(50);
  // turn on voltage regulator (VC=1, VR=1, VF=0)
  sendCommand(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  delay(50);
  // turn on voltage follower (VC=1, VR=1, VF=1)
  sendCommand(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  delay(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  sendCommand(CMD_SET_RESISTOR_RATIO | 0x6);

  // initial display line
  // set page address
  // set column address
  // write display data
  sendCommand(CMD_DISPLAY_ON);
  sendCommand(CMD_SET_ALLPTS_NORMAL);
  setContrast(0x18);

  // set up a bounding box for screen updates
  updateBoundingBox(0, 0, DISPLAY_WIDTH_MAX, DISPLAY_HEIGHT_MAX);
}

void ST7565::sendCommand(uint8_t c)
{
  digitalWrite(a0, LOW);
  spiWrite(c);
}

void ST7565::sendData(uint8_t c)
{
  digitalWrite(a0, HIGH);
  spiWrite(c);
}

inline void ST7565::spiWrite(uint8_t data)
{
  shiftOut(this->sid, this->sclk, MSBFIRST, data);
}

void ST7565::updateBoundingBox(int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax)
{
#ifdef ENABLE_PARTIAL_UPDATE
  if (xmin < this->xUpdateMin)
    this->xUpdateMin = xmin;
  if (xmax > this->xUpdateMax)
    this->xUpdateMax = xmax;
  if (ymin < this->yUpdateMin)
    this->yUpdateMin = ymin;
  if (ymax > this->yUpdateMax)
    this->yUpdateMax = ymax;
#endif
}

void inline ST7565::drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *data, uint16_t offset, uint16_t bytesInData)
{
  if (width < 0 || height < 0)
    return;
  if (yMove + height < 0 || yMove > DISPLAY_HEIGHT)
    return;
  if (xMove + width < 0 || xMove > DISPLAY_WIDTH)
    return;

  uint8_t rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t yOffset = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  int16_t initYMove = yMove;
  int8_t initYOffset = yOffset;

  for (uint16_t i = 0; i < bytesInData; i++)
  {

    // Reset if next horizontal drawing phase is started.
    if (i % rasterHeight == 0)
    {
      yMove = initYMove;
      yOffset = initYOffset;
    }

    uint8_t currentByte = pgm_read_byte(data + offset + i);

    int16_t xPos = xMove + (i / rasterHeight);
    int16_t yPos = ((yMove >> 3) + (i % rasterHeight)) * DISPLAY_WIDTH;

    // int16_t yScreenPos = yMove + yOffset;
    int16_t dataPos = xPos + yPos;

    if (dataPos >= 0 && dataPos < DISPLAY_BUFFER_SIZE &&
        xPos >= 0 && xPos < DISPLAY_WIDTH)
    {

      if (yOffset >= 0)
      {
        switch (this->color)
        {
        case WHITE:
          buffer[dataPos] &= ~(currentByte << yOffset);
          break;
        case BLACK:
          buffer[dataPos] |= currentByte << yOffset;
          break;
        case INVERSE:
          buffer[dataPos] ^= currentByte << yOffset;
          break;
        }
        if (dataPos < (DISPLAY_BUFFER_SIZE - DISPLAY_WIDTH))
        {
          switch (this->color)
          {
          case WHITE:
            buffer[dataPos + DISPLAY_WIDTH] &= ~(currentByte >> (8 - yOffset));
            break;
          case BLACK:
            buffer[dataPos + DISPLAY_WIDTH] |= currentByte >> (8 - yOffset);
            break;
          case INVERSE:
            buffer[dataPos + DISPLAY_WIDTH] ^= currentByte >> (8 - yOffset);
            break;
          }
        }
      }
      else
      {
        // Make new offset position
        yOffset = -yOffset;

        switch (this->color)
        {
        case WHITE:
          buffer[dataPos] &= ~(currentByte >> yOffset);
          break;
        case BLACK:
          buffer[dataPos] |= currentByte >> yOffset;
          break;
        case INVERSE:
          buffer[dataPos] ^= currentByte >> yOffset;
          break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }
    }
  }
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
uint8_t ST7565::utf8ascii(uint8_t ascii)
{
  static uint8_t LASTCHAR;

  if (ascii < 128)
  { // Standard ASCII-set 0..0x7F handling
    LASTCHAR = 0;
    return ascii;
  }

  uint8_t last = LASTCHAR; // get last char
  LASTCHAR = ascii;

  switch (last)
  { // conversion depnding on first UTF8-character
  case 0xC2:
    return (ascii);
    break;
  case 0xC3:
    return (ascii | 0xC0);
    break;
  case 0x82:
    if (ascii == 0xAC)
      return (0x80); // special case Euro-symbol
  }

  return 0; // otherwise: return zero, if character has to be ignored
}

// You need to free the char!
char *ST7565::utf8ascii(String str)
{
  uint16_t k = 0;
  uint16_t length = str.length() + 1;

  // Copy the string into a char array
  char *s = (char *)malloc(length * sizeof(char));
  if (!s)
  {
    cout << "[ST7565][utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n";
    return (char *)str.c_str();
  }
  str.toCharArray(s, length);

  length--;

  for (uint16_t i = 0; i < length; i++)
  {
    char c = utf8ascii(s[i]);
    if (c != 0)
    {
      s[k++] = c;
    }
  }

  s[k] = 0;

  // This will leak 's' be sure to free it in the calling function.
  return s;
}

// END: Private Method
// ------------------------------------------------------------------------------------------------

void ST7565::config(uint8_t contrast, bool negative, bool rotation, bool mirror, uint8_t resistor_ratio)
{
  sendCommand(CMD_SET_VOLUME_FIRST);
  sendCommand(CMD_SET_VOLUME_SECOND | contrast);
  sendCommand(negative ? CMD_SET_DISP_REVERSE : CMD_SET_DISP_NORMAL);
  sendCommand(rotation ? CMD_SET_COM_REVERSE : CMD_SET_COM_NORMAL);
  sendCommand(mirror ? CMD_SET_ADC_REVERSE : CMD_SET_ADC_NORMAL);

  // tỷ lệ điện trở R1/R2, điều chỉnh điện áp hoạt động của lcd với biến val trong khoảng từ 0x0 đến 0x6
  sendCommand(CMD_SET_RESISTOR_RATIO | resistor_ratio);
}

void ST7565::display(void)
{
  uint8_t col, maxcol, p;

#ifdef ENABLE_PARTIAL_UPDATE
  cout << "Refresh LT(" << this->xUpdateMin;
  cout << ", " << this->yUpdateMin;
  cout << ") BR(" << this->xUpdateMax;
  cout << ", " << this->yUpdateMax << ")\n";
#endif

  for (p = 0; p < 8; p++)
  {
#ifdef ENABLE_PARTIAL_UPDATE
    // check if this page is part of update
    if (this->yUpdateMin >= ((p + 1) * 8))
    {
      continue; // nope, skip it!
    }
    if (this->yUpdateMax < p * 8)
    {
      break;
    }
#endif

    sendCommand(CMD_SET_PAGE | pagemap[p]);

#ifdef ENABLE_PARTIAL_UPDATE
    col = this->xUpdateMin;
    maxcol = this->xUpdateMax;
#else
    // start at the beginning of the row
    col = 0;
    maxcol = DISPLAY_WIDTH_MAX;
#endif

    sendCommand(CMD_SET_COLUMN_LOWER | ((col + ST7565_STARTBYTES) & 0xf));
    sendCommand(CMD_SET_COLUMN_UPPER | (((col + ST7565_STARTBYTES) >> 4) & 0x0F));
    sendCommand(CMD_RMW);

    for (; col <= maxcol; col++)
    {
      //uart_putw_dec(col);
      //uart_putchar(' ');
      sendData(this->buffer[(DISPLAY_WIDTH * p) + col]);
    }
  }

#ifdef ENABLE_PARTIAL_UPDATE
  this->xUpdateMin = DISPLAY_WIDTH_MAX;
  this->xUpdateMax = 0;
  this->yUpdateMin = DISPLAY_HEIGHT_MAX;
  this->yUpdateMax = 0;
#endif
}

// Clear the local pixel buffer
void ST7565::clear(void)
{
  memset(this->buffer, 0, DISPLAY_BUFFER_SIZE);
  updateBoundingBox(0, 0, DISPLAY_WIDTH_MAX, DISPLAY_HEIGHT_MAX);
}

// This doesnt touch the buffer, just clears the display RAM - might be handy
void ST7565::clearDisplay(void)
{
  uint8_t p, c;

  for (p = 0; p < 8; p++)
  {
    sendCommand(CMD_SET_PAGE | p);
    for (c = 0; c <= DISPLAY_WIDTH; c++)
    {
      sendCommand(CMD_SET_COLUMN_LOWER | (c & 0xf));
      sendCommand(CMD_SET_COLUMN_UPPER | ((c >> 4) & 0xf));
      sendData(0x0);
    }
  }
}

// ------------------------------------------------------------------------------------------------
// Display Functions

void ST7565::displayOn(void)
{
  sendCommand(CMD_DISPLAY_ON);
  sendCommand(CMD_SET_ALLPTS_NORMAL);
}

void ST7565::displayOff(void)
{
  sendCommand(CMD_DISPLAY_OFF);
}

void ST7565::normalDisplay(void)
{
  sendCommand(CMD_SET_DISP_NORMAL);
}

void ST7565::setContrast(uint8_t contrast)
{
  sendCommand(CMD_SET_VOLUME_FIRST);
  sendCommand(CMD_SET_VOLUME_SECOND | (contrast & 0x3f));
}
// ------------------------------------------------------------------------------------------------
void ST7565::setColor(DISPLAY_COLOR color)
{
  this->color = color;
}

// the most basic function, set a single pixel
void ST7565::setPixelInternal(int16_t x, int16_t y)
{
  if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT)
  {
    // Division
    // y / 8; // normal
    // y >> 3; // bitwise [8 = 2^3, so use 3]
    // Modulus
    // y % 8; // normal
    // y & 7; // bitwise [8 = 1 << 3, apply ((1 << 3) - 1), so use 7]
    switch (this->color)
    {
    case WHITE:
      this->buffer[x + (y >> 3) * DISPLAY_WIDTH] &= ~_BV(7 - (y & 7));
      break;
    case BLACK:
      this->buffer[x + (y >> 3) * DISPLAY_WIDTH] |= _BV(7 - (y & 7));
      break;
    case INVERSE:
      this->buffer[x + (y >> 3) * DISPLAY_WIDTH] ^= _BV(7 - (y & 7));
      break;
    }
  }
}
// the most basic function, set a single pixel
void ST7565::setPixel(int16_t x, int16_t y)
{
  setPixelInternal(x, y);

  updateBoundingBox(x, y, x, y);
}

// the most basic function, get a single pixel
uint8_t ST7565::getPixel(int16_t x, int16_t y)
{
  if ((x >= DISPLAY_WIDTH) || (y >= DISPLAY_HEIGHT))
    return 0;

  return (this->buffer[x + (y >> 3) * DISPLAY_WIDTH] >> (7 - (y & 3))) & 0x1;
}

// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void ST7565::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      setPixel(y0, x0);
    }
    else
    {
      setPixel(x0, y0);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void ST7565::drawRect(int16_t x, int16_t y, int16_t width, int16_t height)
{
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width - 1, y, height);
  drawHorizontalLine(x, y + height - 1, width);
}

void ST7565::fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height)
{
  for (int16_t x = xMove; x < xMove + width; x++)
  {
    drawVerticalLine(x, yMove, height);
  }
}

void ST7565::drawCircle(int16_t x0, int16_t y0, int16_t radius)
{
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  do
  {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;

    setPixel(x0 + x, y0 + y); //For the 8 octants
    setPixel(x0 - x, y0 + y);
    setPixel(x0 + x, y0 - y);
    setPixel(x0 - x, y0 - y);
    setPixel(x0 + y, y0 + x);
    setPixel(x0 - y, y0 + x);
    setPixel(x0 + y, y0 - x);
    setPixel(x0 - y, y0 - x);

  } while (x < y);

  setPixel(x0 + radius, y0);
  setPixel(x0, y0 + radius);
  setPixel(x0 - radius, y0);
  setPixel(x0, y0 - radius);
}

void ST7565::drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads)
{
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  while (x < y)
  {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    if (quads & 0x1)
    {
      setPixel(x0 + x, y0 - y);
      setPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2)
    {
      setPixel(x0 - y, y0 - x);
      setPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4)
    {
      setPixel(x0 - y, y0 + x);
      setPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8)
    {
      setPixel(x0 + x, y0 + y);
      setPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8)
  {
    setPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8)
  {
    setPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4)
  {
    setPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2)
  {
    setPixel(x0, y0 - radius);
  }
}

void ST7565::fillCircle(int16_t x0, int16_t y0, int16_t radius)
{
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  do
  {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;

    drawHorizontalLine(x0 - x, y0 - y, 2 * x);
    drawHorizontalLine(x0 - x, y0 + y, 2 * x);
    drawHorizontalLine(x0 - y, y0 - x, 2 * y);
    drawHorizontalLine(x0 - y, y0 + x, 2 * y);

  } while (x < y);
  drawHorizontalLine(x0 - radius, y0, 2 * radius);
}

void ST7565::drawHorizontalLine(int16_t x, int16_t y, int16_t length)
{
  if (y < 0 || y >= DISPLAY_HEIGHT)
  {
    return;
  }

  if (x < 0)
  {
    length += x;
    x = 0;
  }

  if ((x + length) > DISPLAY_WIDTH)
  {
    length = (DISPLAY_WIDTH - x);
  }

  if (length <= 0)
  {
    return;
  }

  updateBoundingBox(x, y, x + length, y);

  uint8_t *bufferPtr = buffer;
  bufferPtr += (x + (y >> 3) * DISPLAY_WIDTH);

  uint8_t drawBit = _BV(7 - (y & 7));

  switch (color)
  {
  case WHITE:
    drawBit = ~drawBit;
    while (length--)
    {
      *bufferPtr++ &= drawBit;
    };
    break;
  case BLACK:
    while (length--)
    {
      *bufferPtr++ |= drawBit;
    };

    break;
  case INVERSE:
    while (length--)
    {
      *bufferPtr++ ^= drawBit;
    };
    break;
  }
}

void ST7565::drawVerticalLine(int16_t x, int16_t y, int16_t length)
{
  if (x < 0 || x >= DISPLAY_WIDTH)
    return;

  if (y < 0)
  {
    length += y;
    y = 0;
  }

  if ((y + length) > DISPLAY_HEIGHT)
  {
    length = (DISPLAY_HEIGHT - y);
  }

  if (length <= 0)
    return;

  updateBoundingBox(x, y, x, y + length);
  // updateBoundingBox(0, 0, DISPLAY_WIDTH_MAX, DISPLAY_HEIGHT_MAX);

  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = buffer;

  bufferPtr += (y >> 3) * DISPLAY_WIDTH;
  bufferPtr += x;

  // cout << x << "," << y << " => " << x << "," << y + length << ", L = " << length << endl;
  // cout << "\tyOffset: " << (int)yOffset << " " << bitset<8>(yOffset) << endl;
  if (yOffset)
  {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF << (yOffset));

    if (length < yOffset)
    {
      drawBit &= (0xFF >> (yOffset - length));
    }

    // cout << "\tC1 Draw " << y << " to " << (int)y + yOffset - 1 << endl;
    // cout << "\t\tdrawBit: " << bitset<8>(drawBit) << endl;

    switch (color)
    {
    case WHITE:
      *bufferPtr &= ~drawBit;
      break;
    case BLACK:
      *bufferPtr |= drawBit;
      break;
    case INVERSE:
      *bufferPtr ^= drawBit;
      break;
    }

    if (length < yOffset)
      return;

    length -= yOffset;
    bufferPtr += DISPLAY_WIDTH;
  }

  // uint8_t old_lenght = length;
  if (length >= 8)
  {
    // cout << "\tC2 Draw " << (int)y + yOffset - 1 << " to " << (int)y + yOffset - 1 + old_lenght << endl;
    switch (color)
    {
    case WHITE:
    case BLACK:
      drawBit = (color == BLACK) ? 0xFF : 0x00;
      do
      {
        // cout << "\t\tdrawBit: " << bitset<8>(drawBit) << endl;
        *bufferPtr = drawBit;
        bufferPtr += DISPLAY_WIDTH;
        length -= 8;
      } while (length >= 8);
      break;
    case INVERSE:
      do
      {
        *bufferPtr = ~(*bufferPtr);
        bufferPtr += DISPLAY_WIDTH;
        length -= 8;
      } while (length >= 8);
      break;
    }
  }

  // cout << "\tC3 Draw " << y + yOffset + old_lenght << " to " << (int)y + yOffset + old_lenght + length << endl;
  if (length > 0)
  {
    drawBit = 0xFF << (8 - length);
    // cout << "\t\tdrawBit: " << (int)drawBit << " " << bitset<8>(drawBit) << endl;
    switch (color)
    {
    case WHITE:
      *bufferPtr &= ~drawBit;
      break;
    case BLACK:
      *bufferPtr |= drawBit;
      break;
    case INVERSE:
      *bufferPtr ^= drawBit;
      break;
    }
  }
}

void ST7565::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress)
{
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
  drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  uint16_t maxProgressWidth = (width - doubleRadius - 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

void ST7565::drawFastImage(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *image)
{
  drawInternal(xMove, yMove, width, height, image, 0, 0);
}

void ST7565::drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *xbm)
{
  int16_t widthInXbm = (width + 7) / 8;
  uint8_t data = 0;

  for (int16_t y = 0; y < height; y++)
  {
    for (int16_t x = 0; x < width; x++)
    {
      if (x & 7)
      {
        data >>= 1; // Move a bit
      }
      else
      { // Read new data every 8 bit
        data = pgm_read_byte(xbm + (x / 8) + y * widthInXbm);
      }
      // if there is a bit draw it
      if (data & 0x01)
      {
        setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void ST7565::drawStringInternal(int16_t xMove, int16_t yMove, char *text, uint16_t textLength, uint16_t textWidth)
{
  uint8_t textHeight = pgm_read_byte(fontData + HEIGHT_POS);
  uint8_t firstChar = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t sizeOfJumpTable = pgm_read_byte(fontData + CHAR_NUM_POS) * JUMPTABLE_BYTES;

  uint8_t cursorX = 0;
  uint8_t cursorY = 0;

  switch (this->textAlignment)
  {
  case TEXT_ALIGN_CENTER_BOTH:
    yMove -= textHeight >> 1;
  // Fallthrough
  case TEXT_ALIGN_CENTER:
    xMove -= textWidth >> 1; // divide by 2
    break;
  case TEXT_ALIGN_RIGHT:
    xMove -= textWidth;
    break;
  case TEXT_ALIGN_LEFT:
    break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth < 0 || xMove > DISPLAY_WIDTH)
  {
    return;
  }
  if (yMove + textHeight < 0 || yMove > DISPLAY_HEIGHT)
  {
    return;
  }

  for (uint16_t j = 0; j < textLength; j++)
  {
    int16_t xPos = xMove + cursorX;
    int16_t yPos = yMove + cursorY;

    uint8_t code = text[j];
    if (code >= firstChar)
    {
      uint8_t charCode = code - firstChar;

      // 4 Bytes per char code
      uint8_t msbJumpToChar = pgm_read_byte(fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES);                      // MSB  \ JumpAddress
      uint8_t lsbJumpToChar = pgm_read_byte(fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);      // LSB /
      uint8_t charByteSize = pgm_read_byte(fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);      // Size
      uint8_t currentCharWidth = pgm_read_byte(fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(msbJumpToChar == 255 && lsbJumpToChar == 255))
      {
        // Get the position of the char data
        uint16_t charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
      }

      cursorX += currentCharWidth;
    }
  }
}

void ST7565::drawString(int16_t xMove, int16_t yMove, String strUser)
{
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  // char* text must be freed!
  char *text = utf8ascii(strUser);

  uint16_t yOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH)
  {
    uint16_t lb = 0;
    // Find number of linebreaks in text
    for (uint16_t i = 0; text[i] != 0; i++)
    {
      lb += (text[i] == 10);
    }
    // Calculate center
    yOffset = (lb * lineHeight) / 2;
  }

  uint16_t line = 0;
  char *textPart = strtok(text, "\n");
  while (textPart != NULL)
  {
    uint16_t length = strlen(textPart);
    drawStringInternal(xMove, yMove - yOffset + (line++) * lineHeight, textPart, length, getStringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }
  free(text);
}

void ST7565::drawStringMaxWidth(int16_t xMove, int16_t yMove, uint16_t maxLineWidth, String strUser)
{
  uint16_t firstChar = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  char *text = utf8ascii(strUser);

  uint16_t length = strlen(text);
  uint16_t lastDrawnPos = 0;
  uint16_t lineNumber = 0;
  uint16_t strWidth = 0;

  uint16_t preferredBreakpoint = 0;
  uint16_t widthAtBreakpoint = 0;

  for (uint16_t i = 0; i < length; i++)
  {
    strWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[i] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (text[i] == ' ' || text[i] == '-')
    {
      preferredBreakpoint = i;
      widthAtBreakpoint = strWidth;
    }

    if (strWidth >= maxLineWidth)
    {
      if (preferredBreakpoint == 0)
      {
        preferredBreakpoint = i;
        widthAtBreakpoint = strWidth;
      }
      drawStringInternal(xMove, yMove + (lineNumber++) * lineHeight, &text[lastDrawnPos], preferredBreakpoint - lastDrawnPos, widthAtBreakpoint);
      lastDrawnPos = preferredBreakpoint + 1;
      // It is possible that we did not draw all letters to i so we need
      // to account for the width of the chars from `i - preferredBreakpoint`
      // by calculating the width we did not draw yet.
      strWidth = strWidth - widthAtBreakpoint;
      preferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (lastDrawnPos < length)
  {
    drawStringInternal(xMove, yMove + lineNumber * lineHeight, &text[lastDrawnPos], length - lastDrawnPos, getStringWidth(&text[lastDrawnPos], length - lastDrawnPos));
  }

  free(text);
}

uint16_t ST7565::getStringWidth(const char *text, uint16_t length)
{
  uint16_t firstChar = pgm_read_byte(fontData + FIRST_CHAR_POS);

  uint16_t stringWidth = 0;
  uint16_t maxWidth = 0;

  while (length--)
  {
    stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10)
    {
      maxWidth = max(maxWidth, stringWidth);
      stringWidth = 0;
    }
  }

  return max(maxWidth, stringWidth);
}

uint16_t ST7565::getStringWidth(String strUser)
{
  char *text = utf8ascii(strUser);
  uint16_t length = strlen(text);
  uint16_t width = getStringWidth(text, length);
  free(text);
  return width;
}

void ST7565::setTextAlignment(DISPLAY_TEXT_ALIGNMENT textAlignment)
{
  this->textAlignment = textAlignment;
}

void ST7565::setFont(const char *fontData)
{
  this->fontData = fontData;
}