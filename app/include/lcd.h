#ifndef _LCD_H_
#define _LCD_H_

// Manages the LCD to change what information is shown on screen

void lcd_changeScreen(int new_screen_mode);

void lcd_init();

void lcd_cleanup();

#endif