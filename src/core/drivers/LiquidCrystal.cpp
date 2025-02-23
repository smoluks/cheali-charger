/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DummyLiquidCrystal_h
#define DummyLiquidCrystal_h

#include "LiquidCrystal.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <HD44780.h>
#include "IO.h"
#include "Utils.h"
#include "Hardware.h"

namespace LiquidCrystal
{
  void send(uint8_t, uint8_t);

  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines, _currline;
}

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).
void LiquidCrystal::init()
{
  begin(16, 1);
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
  _numlines = lines;
  _currline = 0;
  
  HD44780::init(cols, 2, dotsize);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  HD44780::command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

/********** high level commands, for the user! */
void LiquidCrystal::clear()
{
  HD44780::command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
  Utils::delayMicroseconds(2000);     // this command takes a long time!
}

void LiquidCrystal::home()
{
  HD44780::command(LCD_RETURNHOME); // set cursor position to zero
  Utils::delayMicroseconds(2000);   // this command takes a long time!
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  if (row >= _numlines)
  {
    row = _numlines - 1; // we count rows starting w/0
  }

  HD44780::command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay()
{
  _displaycontrol &= ~LCD_DISPLAYON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::display()
{
  _displaycontrol |= LCD_DISPLAYON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor()
{
  _displaycontrol &= ~LCD_CURSORON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::cursor()
{
  _displaycontrol |= LCD_CURSORON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink()
{
  _displaycontrol &= ~LCD_BLINKON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::blink()
{
  _displaycontrol |= LCD_BLINKON;
  HD44780::command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft(void)
{
  HD44780::command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal::scrollDisplayRight(void)
{
  HD44780::command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight(void)
{
  _displaymode |= LCD_ENTRYLEFT;
  HD44780::command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft(void)
{
  _displaymode &= ~LCD_ENTRYLEFT;
  HD44780::command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll(void)
{
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  HD44780::command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll(void)
{
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  HD44780::command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  HD44780::command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++)
  {
    HD44780::write(charmap[i]);
  }
}

void LiquidCrystal::write(const uint8_t *buffer, uint8_t size)
{
  while (size--)
  {
    HD44780::write(*buffer++);
  }
}

void LiquidCrystal::print(const char str[])
{
  while (true)
  {
    char c = *str++;
    if (c == 0)
      break;

    HD44780::write(c);
  }
}

void LiquidCrystal::print(char c)
{
  HD44780::write((uint8_t)c);
}

#endif