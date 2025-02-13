#include "LCD.h"
#include "adc_stm8.h"
#include "delay.h" // Zajisti, že máš správný STM8 hlavičkový soubor
#include "stm8s.h"

void LCD_EnablePulse(void) {
    GPIO_WriteHigh(LCD_EN_PORT, LCD_EN_PIN); // Enable na HIGH
    delay_us(1);                             // Krátké zpoždění
    GPIO_WriteLow(LCD_EN_PORT, LCD_EN_PIN);  // Enable na LOW
    delay_us(1);                             // Další zpoždění
}

void LCD_Send4Bits(unsigned char data) {
    // Zapiš bity do pinů D4 až D7
    if (data & 0x01)
        GPIO_WriteHigh(LCD_D4_PORT, LCD_D4_PIN);
    else
        GPIO_WriteLow(LCD_D4_PORT, LCD_D4_PIN);
    if (data & 0x02)
        GPIO_WriteHigh(LCD_D5_PORT, LCD_D5_PIN);
    else
        GPIO_WriteLow(LCD_D5_PORT, LCD_D5_PIN);
    if (data & 0x04)
        GPIO_WriteHigh(LCD_D6_PORT, LCD_D6_PIN);
    else
        GPIO_WriteLow(LCD_D6_PORT, LCD_D6_PIN);
    if (data & 0x08)
        GPIO_WriteHigh(LCD_D7_PORT, LCD_D7_PIN);
    else
        GPIO_WriteLow(LCD_D7_PORT, LCD_D7_PIN);

    LCD_EnablePulse();
}

void LCD_Command(unsigned char cmd) {
    GPIO_WriteLow(LCD_RS_PORT, LCD_RS_PIN); // RS na LOW (příkaz)
    LCD_Send4Bits(cmd >> 4);                // Horní 4 bity
    LCD_Send4Bits(cmd & 0x0F);              // Spodní 4 bity
}

void LCD_Char(unsigned char data) {
    GPIO_WriteHigh(LCD_RS_PORT, LCD_RS_PIN); // RS na HIGH (data)
    LCD_Send4Bits(data >> 4);                // Horní 4 bity
    LCD_Send4Bits(data & 0x0F);              // Spodní 4 bity
}

void LCD_Backlight(bool value) {
    if (value) {

        GPIO_WriteHigh(LCD_BL_PORT, LCD_BL_PIN);
    } else {
        GPIO_WriteLow(LCD_BL_PORT, LCD_BL_PIN);
    }
}

void LCD_Init(void) {
    // Inicializace GPIO pinů
    GPIO_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_EN_PORT, LCD_EN_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(LCD_BL_PORT, LCD_BL_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_WriteHigh(LCD_BL_PORT, LCD_BL_PIN);
    delay_ms(20); // Počáteční zpoždění po zapnutí

    // Sekvence inicializace displeje pro 4bitový režim
    LCD_Send4Bits(0x03); // Reset displeje
    delay_ms(5);
    LCD_Send4Bits(0x03); // Zopakuj
    delay_us(100);
    LCD_Send4Bits(0x03); // Zopakuj
    delay_ms(1);
    LCD_Send4Bits(0x02); // Nastav 4bitový režim

    // Konfigurace displeje
    LCD_Command(0x28); // 4bitový režim, 2 řádky, 5x8 znaková matice
    LCD_Command(0x0C); // Zapnutí displeje, bez kurzoru
    LCD_Command(0x06); // Automatický posun kurzoru doprava
    LCD_Command(0x01); // Vymaž displej
    delay_ms(2);
}

void LCD_Button_Init() {
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL5, DISABLE);

    ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);

    ADC2_AlignConfig(ADC2_ALIGN_RIGHT);

    ADC2_Select_Channel(ADC2_CHANNEL_5);
    // rozběhneme AD převodník
    ADC2_Cmd(ENABLE);

    ADC2_Startup_Wait();
}

char LCD_Button_Get() {
    uint16_t N = ADC_get(ADC2_CHANNEL_5);
    char result;
    if (N < 20) {
        result = 'r';
    } else if (N < 160) {
        result = 'u';
    } else if (N < 400) {
        result = 'd';
    } else if (N < 700) {
        result = 'l';
    } else if (N < 1000) {
        result = 's';
    } else {
        result = 'n';
    }
    return result;
}

void LCD_Clear(void) {
    LCD_Command(0x01); // Vymaž displej
    delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    unsigned char pos;
    if (row == 0) {
        pos = 0x80 + col; // První řádek
    } else {
        pos = 0xC0 + col; // Druhý řádek
    }
    LCD_Command(pos);
}

void LCD_String(const char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void LCD_CreateCustomChar(unsigned char location, unsigned char charmap[]) {
    location &= 0x07; // Omezíme na rozsah 0–7, protože máme 8 pozic v CGRAM
    LCD_Command(0x40 | (location << 3)); // Nastavení adresy v CGRAM

    for (int i = 0; i < 8; i++) {
        LCD_Char(charmap[i]); // Zapiš data pro každý řádek znaku (5x8 matice)
    }
}
