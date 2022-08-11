#include "lcd2004a.h"

// By default these three pins are high so the address by default is 01001110 which is 0x4E.
#define SLAVE_ADDRESS_LCD 0x4E

extern I2C_HandleTypeDef hi2c1;
HAL_StatusTypeDef ret;

typedef struct
{
	uint8_t column;
	uint8_t row;
}Cursor;

Cursor cursor;

//	Los comandos son de 8 bits pero se dividiran en dos nibles[c7 c6 c5 c4] + [c3 c2 c1 c0]
//
// A cada nibble del comado  se le deben agregar el siguiente nible: [BL EN RW RS]
// BL: Backlight, 1 encendido y 0 apagado, asi es el backlight se controlad digitalmente.
// EN: enable, se debe colcoar a 1 cuando se transmite un nuevo comando o dato, al terminar pasar a cero, por esta razon se debe enviar dos veces
// RW: seleccion de escribir o leer, 0 para escribir 1 para leer
// RS: seleccion de registros, 0 para comandos y 1 para datos
void lcd_send_cmd(char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C;  // inicio transmision del primer nible -> [c7 c6 c5 c4] + [BL=1 EN=1 RW=0 RS=0]
	data_t[1] = data_u | 0x08;  // finalizo transmision del primer nible -> [c7 c6 c5 c4] + [BL=1 EN=0 RW=0 RS=0]
	data_t[2] = data_l | 0x0C;  // inicio transmision del segundo nible -> [c7 c6 c5 c4] + [BL=1 EN=1 RW=0 RS=0]
	data_t[3] = data_l | 0x08;  // finalizo transmision del segundo nible -> [c7 c6 c5 c4] + [BL=1 EN=0 RW=0 RS=0]
	HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t*) data_t, 4, 1000);
}

// Muy similar a la explicacion del comandos, la linea RS ahora siempre en 1.
// Los datos son caracteres de bits pero se dividiran en dos nibles[d7 d6 d5 d4] + [d3 d2 d1 d0]
void lcd_send_data(char data)
{
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;
	data_t[1] = data_u | 0x09;
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;
	HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t*) data_t, 4, 1000);
}

void lcd_clear()
{
	lcd_send_cmd(0x80);
	for (int i = 0; i < 80; i++)
	{
		lcd_send_data(' ');
	}
	cursor.column = 1;
	cursor.row = 1;
	lcd_move_cursor();
}

void lcd_move_cursor()
{
	int cmd;

	cmd = 0x80; //cursor.column;

	switch (cursor.row)
	{
	case 1:
		cmd |= 0x00;
		break;
	case 2:
		cmd |= 0x40;
		break;
	case 3:
		cmd |= 0x14;
		break;
	case 4:
		cmd |= 0x54;
		break;
	default:
		return;
		break;
	}
	lcd_send_cmd(cmd + cursor.column - 1);
}

void lcd_2004_init(void)
{
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd(0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd(0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd(0x30);
	HAL_Delay(10);
	lcd_send_cmd(0x20);  // 4bit mode
	HAL_Delay(10);

	// dislay initialisation
	lcd_send_cmd(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd(0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd(0x01);  // clear display
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd(0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd(0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)

	lcd_clear();
}

void lcd_send_string(char *str)
{
	while (*str)
	{
		if (*str > ' ' && *str < '~')
		{
			lcd_send_data(*str);
			cursor.column++;
			if(cursor.column > 20)
			{
				cursor.column = 1;
				lcd_move_cursor();
			}
		}
		else if (*str == '\n')
		{
			cursor.column  = 1;
			cursor.row += 1;
			if(cursor.row > 4)
			{
				cursor.row = 1;
			}
			lcd_move_cursor();
		}
		else if (*str == '\r')
		{
			cursor.column = 1;
			lcd_move_cursor();
		}
		else
		{
			// Caracter no esperado
		}
		str++;
	}
}
