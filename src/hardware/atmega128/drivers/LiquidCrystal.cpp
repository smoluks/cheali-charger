#include "LiquidCrystal.h"
#include "Utils.h"

namespace LiquidCrystal
{
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _initialized;
  uint8_t _numlines, _currline;

  void begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
  {
    _numlines = lines;
    _currline = 0;
    
    HD44780::init(cols, lines, dotsize);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    HD44780::sendCommand(LCD_ENTRYMODESET | _displaymode);
  }

  /********** high level commands, for the user! */
  void clear()
  {
    HD44780::sendCommand(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
    Utils::delayMicroseconds(2000);         // this command takes a long time!
  }

  void home()
  {
    HD44780::sendCommand(LCD_RETURNHOME); // set cursor position to zero
    Utils::delayMicroseconds(2000);       // this command takes a long time!
  }

  void setCursor(uint8_t col, uint8_t row)
  {
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= _numlines)
    {
      row = _numlines - 1; // we count rows starting w/0
    }

    HD44780::sendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
  }

  // Turn the display on/off (quickly)
  void noDisplay()
  {
    _displaycontrol &= ~LCD_DISPLAYON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }
  void display()
  {
    _displaycontrol |= LCD_DISPLAYON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }

  // Turns the underline cursor on/off
  void noCursor()
  {
    _displaycontrol &= ~LCD_CURSORON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }
  void cursor()
  {
    _displaycontrol |= LCD_CURSORON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }

  // Turn on and off the blinking cursor
  void noBlink()
  {
    _displaycontrol &= ~LCD_BLINKON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }
  void blink()
  {
    _displaycontrol |= LCD_BLINKON;
    HD44780::sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
  }

  // These commands scroll the display without changing the RAM
  void scrollDisplayLeft(void)
  {
    HD44780::sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
  }
  void scrollDisplayRight(void)
  {
    HD44780::sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
  }

  // This is for text that flows Left to Right
  void leftToRight(void)
  {
    _displaymode |= LCD_ENTRYLEFT;
    HD44780::sendCommand(LCD_ENTRYMODESET | _displaymode);
  }

  // This is for text that flows Right to Left
  void rightToLeft(void)
  {
    _displaymode &= ~LCD_ENTRYLEFT;
    HD44780::sendCommand(LCD_ENTRYMODESET | _displaymode);
  }

  // This will 'right justify' text from the cursor
  void autoscroll(void)
  {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    HD44780::sendCommand(LCD_ENTRYMODESET | _displaymode);
  }

  // This will 'left justify' text from the cursor
  void noAutoscroll(void)
  {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    HD44780::sendCommand(LCD_ENTRYMODESET | _displaymode);
  }

  // Allows us to fill the first 8 CGRAM locations
  // with custom characters
  void createChar(uint8_t location, uint8_t charmap[])
  {
    location &= 0x7; // we only have 8 locations 0-7
    HD44780::sendCommand(LCD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++)
    {
      HD44780::sendData(charmap[i]);
    }
  }

  /*uint8_t print(const char str[])
  {
    return write(str);
  }*/

  uint8_t print(char c)
  {
    HD44780::sendData((uint8_t)c);
    return 0;
  }

  /*uint8_t write(const uint8_t *buffer, uint8_t size)
  {
    uint8_t n = 0;
    while (size--)
    {
      n += write(*buffer++);
    }
    return n;
  }*/
}
