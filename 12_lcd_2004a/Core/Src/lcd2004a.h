#ifndef I2C_LCD_2004_H
#define I2C_LCD_2004_H

#include "main.h"

// Los siguientes defines hacen uso de los recursos de microctontrolador,
// en el proyecto final es buena idea moverlos a un archivo donde esten
// todos juntos los recuersos del microcontrolador

void lcd_2004_init ();   // initialize lcd
void lcd_send_cmd (char cmd);  // send command to the lcd
void lcd_send_data (char data);  // send data to the lcd
void lcd_send_string (char *str);  // send string to the lcd
void lcd_move_cursor();  // put cursor at the entered position row (0 or 1), col (0-15);
void lcd_clear (void);

#endif
