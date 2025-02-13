#ifndef LCD_H
#define LCD_H

// Definuj si porty a piny pro jednotlivé signály
#define LCD_RS_PORT GPIOD     // Port pro RS pin
#define LCD_RS_PIN GPIO_PIN_3 // RS pin

#define LCD_EN_PORT GPIOC     // Port pro EN pin
#define LCD_EN_PIN GPIO_PIN_4 // Enable pin

#define LCD_D4_PORT GPIOG     // Port pro D4 pin
#define LCD_D4_PIN GPIO_PIN_0 // Data pin D4

#define LCD_D5_PORT GPIOC     // Port pro D5 pin
#define LCD_D5_PIN GPIO_PIN_2 // Data pin D5

#define LCD_D6_PORT GPIOC     // Port pro D6 pin
#define LCD_D6_PIN GPIO_PIN_3 // Data pin D6

#define LCD_D7_PIN GPIO_PIN_1
#define LCD_D7_PORT GPIOD // Data pin D7

#define LCD_BL_PIN GPIO_PIN_5
#define LCD_BL_PORT GPIOE
// Prototypy funkcí
void LCD_Init(void);
void LCD_Backlight(bool value);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_CreateCustomChar(unsigned char location, unsigned char charmap[]);
void LCD_Button_Init();
char LCD_Button_Get();

#endif
