#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <stdint.h>
#include <string.h>

#include "HD44780.h"

namespace LiquidCrystal {
  /*inline uint8_t write(const char *str)
  {
    if (str == 0)
      return 0;

    return write((const uint8_t *)str, strlen(str));
  }*/

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);

  //uint8_t write(const uint8_t *buffer, uint8_t size);
  uint8_t print(char c);
  //uint8_t print(const char buffer[]);
} //namespace LiquidCrystal

#endif