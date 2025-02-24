/*
    HD44780 all-port driver for cheali-charger
*/
#include "HD44780.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "IO.h"
#include "Utils.h"
#include "Hardware.h"

namespace HD44780
{

#ifdef LCD_ENABLE_8BITMODE
  void write8bits(uint8_t);
#else
  void write4bits(uint8_t);
#endif // LCD_ENABLE_8BITMODE

  void pulseEnable(void);
  void waitReady(void);

  void init(uint8_t cols, uint8_t lines, uint8_t dotsize)
  {
    uint8_t _displayfunction;
#ifndef LCD_ENABLE_8BITMODE
    _displayfunction = LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
#else
    _displayfunction = LCD_FUNCTIONSET | LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
#endif // LCD_ENABLE_8BITMODE

    if (lines > 1)
    {
      _displayfunction |= LCD_2LINE;
    }

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1))
    {
      _displayfunction |= LCD_5x10DOTS;
    }

    // ----- configure gpio ------
    IO::setIO(LCD_RS_DDR);
    IO::setIO(LCD_E_DDR);

    IO::resetIO(LCD_RS_PORT);
    IO::resetIO(LCD_E_PORT);

    // we can save 1 pin by not using RW, but what cost?
#ifdef LCD_RW_PORT
    IO::setIO(LCD_RW_DDR);
    IO::resetIO(LCD_RW_PORT);
#endif // LCD_RW_PORT

#ifndef LCD_ENABLE_8BITMODE
    IO::setHighHalfPort(LCD_DATA_DDR, 0xFF);
#else
    IO::setPort(LCD_DATA_DDR, PIN_ALL);
#endif // LCD_ENABLE_8BITMODE

    //----- initialization ------
    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    Utils::delayMicroseconds(50000);

    // put the LCD into 4 bit or 8 bit mode
#ifndef LCD_ENABLE_8BITMODE
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    Utils::delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    Utils::delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    Utils::delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02);
#else
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    sendCommandNoWait(_displayfunction);
    Utils::delayMicroseconds(4100); // wait more than 4.1ms

    // second try
    sendCommandNoWait(_displayfunction);
    Utils::delayMicroseconds(100);

    // third go
    command(_displayfunction);
#endif // LCD_ENABLE_8BITMODE

    // finally, set # lines, font size, etc.
    HD44780::command(_displayfunction);
  }

  void command(uint8_t value)
  {
    IO::resetIO(LCD_RS_PORT);

    // if there is a RW pin indicated, set it low to Write
#ifdef LCD_RW_PORT
    IO::resetIO(LCD_RW_PORT);
#endif

#ifdef LCD_ENABLE_8BITMODE
    write8bits(value);
    pulseEnable();
    waitReady();
#else
    write4bits(value);
    pulseEnable();
    waitReady();
    write4bits(value << 4);
    pulseEnable();
    waitReady();
#endif // LCD_ENABLE_8BITMODE
  }

  void sendCommandNoWait(uint8_t value)
  {
    IO::resetIO(LCD_RS_PORT);

    // if there is a RW pin indicated, set it low to Write
#ifdef LCD_RW_PORT
    IO::resetIO(LCD_RW_PORT);
#endif

#ifdef LCD_ENABLE_8BITMODE
    write8bits(value);
    pulseEnable();
    Utils::delayMicroseconds(100);
#else
    write4bits(value);
    pulseEnable();
    Utils::delayMicroseconds(100);
    write4bits(value << 4);
    pulseEnable();
    Utils::delayMicroseconds(100);
#endif // LCD_ENABLE_8BITMODE
  }

  void write(uint8_t value)
  {
    IO::setIO(LCD_RS_PORT);

    // if there is a RW pin indicated, set it low to Write
#ifdef LCD_RW_PORT
    IO::resetIO(LCD_RW_PORT);
#endif

#ifdef LCD_ENABLE_8BITMODE
    write8bits(value);
    pulseEnable();
    waitReady();
#else
    write4bits(value);
    pulseEnable();
    waitReady();
    write4bits(value << 4);
    pulseEnable();
    waitReady();
#endif // LCD_ENABLE_8BITMODE
  }

#ifdef LCD_ENABLE_8BITMODE
  void write8bits(uint8_t value)
  {
    IO::setPort(LCD_DATA_PORT, value);
  }
#else
  void write4bits(uint8_t value)
  {
    IO::setHighHalfPort(LCD_DATA_PORT, value);
  }
#endif // LCD_ENABLE_8BITMODE

  void pulseEnable(void)
  {
    __nop();
    IO::setIO(LCD_E_PORT);
    __nop(); // 230ns max at 5V, see page 52
    __nop();
    __nop();
    __nop();
    __nop();
    IO::resetIO(LCD_E_PORT);
  }

  void waitReady(void)
  {
#ifndef LCD_RW_PORT
    Utils::delayMicroseconds(100);
#else

#ifndef LCD_ENABLE_8BITMODE
    IO::setHighHalfPort(LCD_DATA_DDR, 0);
#else
    IO::setPort(LCD_DATA_DDR, 0);
#endif

    IO::setIO(LCD_RW_PORT);
    IO::resetIO(LCD_RS_PORT);

    while (true)
    {
#ifdef LCD_ENABLE_8BITMODE
      IO::setIO(LCD_E_PORT);
      __nop(); // 160ns max at 5V, see page 52
      __nop();
      __nop();

      uint8_t busyFlag = IO::readIOBit(LCD_DATA_PIN, 7);

      IO::resetIO(LCD_E_PORT);

      if (!busyFlag)
        break;
#else
      IO::setIO(LCD_E_PORT);
      __nop();
      __nop();
      __nop();
      uint8_t busyFlag = IO::readIOBit(LCD_DATA_PIN, 7);
      IO::resetIO(LCD_E_PORT);
      __nop();
      __nop();
      __nop();
      IO::setIO(LCD_E_PORT);
      __nop();
      __nop();
      __nop();
      IO::resetIO(LCD_E_PORT);

      if (!busyFlag)
        break;
#endif // LCD_ENABLE_8BITMODE
    }

#ifndef LCD_ENABLE_8BITMODE
    IO::setHighHalfPort(LCD_DATA_DDR, 0xFF);
#else
    IO::setPort(LCD_DATA_DDR, 0xFF);
#endif

#endif // LCD_RW_PORT
  }
}