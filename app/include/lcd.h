#ifndef _LCD_H_
#define _LCD_H_

#include <stdbool.h>

// Manages the LCD to change what information is shown on screen

void lcd_wakeScreen(void);

void lcd_init();

void lcd_cleanup();

#endif