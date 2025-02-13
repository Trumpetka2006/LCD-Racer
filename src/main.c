#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm8s.h>
// #include <stdio.h>
#include "LCD.h"

#include "delay.h"
#include "main.h"
#include "milis.h"
// #include "uart1.h"
//
//
// High Score: 1703 - Já

#define HIGHSCORE_LOW_ADDR 0x4001
#define HIGHSCORE_HIGH_ADDR 0x4000

const unsigned char logoA[] = {0b00000, 0b00011, 0b00100, 0b01000,
                               0b11111, 0b01000, 0b00100, 0b00011};
const unsigned char logoB[] = {0b11111, 0b00100, 0b01100, 0b10100,
                               0b00101, 0b00110, 0b00100, 0b11111};
const unsigned char logoC[] = {0b00000, 0b11000, 0b00100, 0b00010,
                               0b11111, 0b00010, 0b00100, 0b11000};

const unsigned char carA[] = {0b00000, 0b00000, 0b00111, 0b01101,
                              0b11111, 0b11111, 0b01100, 0b00000};

const unsigned char carB[] = {0b00000, 0b00000, 0b11000, 0b00000,
                              0b11111, 0b11111, 0b00110, 0b00000};

void init(void) {
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovani MCU na 16MHz

    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
#if defined(BTN_PORT) || defined(BTN_PIN)
    GPIO_Init(BTN_PORT, BTN_PIN, GPIO_MODE_IN_FL_NO_IT);
#endif

    init_milis();
    // init_uart1();
}

void usart_puts(char *Buffer) {
    while (*Buffer) { // než narazíš na konec řetězce (znak /0)
        while (!UART3_GetFlagStatus(UART3_FLAG_TXE)) {
        }; // čekej než bude volno v Tx Bufferu
        UART3_SendData8(*Buffer++); // předej znak k odeslání
    }
}

void splitUint16(uint16_t input, uint8_t *highByte, uint8_t *lowByte) {
    *highByte = (input >> 8) & 0xFF; // Horní byte
    *lowByte = input & 0xFF;         // Dolní byte
}

uint16_t mergeUint8(uint8_t highByte, uint8_t lowByte) {
    return ((uint16_t)highByte << 8) | (uint16_t)lowByte;
}

void writeEEPROM(uint16_t address, uint8_t data) {
    // Odemknutí EEPROM
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

    // Zápis dat na adresu
    FLASH_ProgramByte(address, data);

    // Zamknutí EEPROM
    FLASH_Lock(FLASH_MEMTYPE_DATA);
}

uint8_t readEEPROM(uint16_t address) {
    // Přímé čtení z paměti
    return *((uint8_t *)address);
}

void credits() {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("Jiri Gres 4L");
    delay_ms(1000);
    char btn = 'n';
    while (1) {
        btn = LCD_Button_Get();
        if (btn == 's') {
            break;
        } else if (btn == 'd') {

            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_String("Ereasing...");
            writeEEPROM(HIGHSCORE_HIGH_ADDR, 0x00);
            writeEEPROM(HIGHSCORE_LOW_ADDR, 0x00);
            delay_ms(2000);
        }
        delay_ms(500);
    }
}

void game_over(uint16_t score) {
    uint8_t Highscore_high = readEEPROM(HIGHSCORE_HIGH_ADDR);
    uint8_t Highscore_low = readEEPROM(HIGHSCORE_LOW_ADDR);

    uint16_t Highscore = mergeUint8(Highscore_high, Highscore_low);

    char String0[16];
    char String1[16];
    sprintf(String0, "Score: %u", score);
    sprintf(String1, "Hi-score: %u", Highscore);

    if (score > Highscore) {

        splitUint16(score, &Highscore_high, &Highscore_low);
        writeEEPROM(HIGHSCORE_HIGH_ADDR, Highscore_high);
        writeEEPROM(HIGHSCORE_LOW_ADDR, Highscore_low);
        sprintf(String1, "New Best!");
    }

    for (int x = 0; x < 5; x++) {
        LCD_Backlight(0);
        delay_ms(200);
        LCD_Backlight(1);
        delay_ms(200);
    }
    LCD_SetCursor(0, 0);
    LCD_String("****-<GAME>-****");
    LCD_SetCursor(1, 0);
    LCD_String("****-<OVER>-****");
    delay_ms(2000);
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String(String0);
    delay_ms(1500);
    LCD_SetCursor(1, 0);
    LCD_String(String1);

    while (LCD_Button_Get() == 'n') {
    }
}

void game_tick(bool lane, char *line1, char *line2) {
    if (lane) {
        line2[1] = 3;
        line2[2] = 4;

        line1[1] = ' ';
        line1[2] = ' ';

    } else {

        line1[1] = 3;
        line1[2] = 4;

        line2[1] = ' ';
        line2[2] = ' ';
    }
}
void extractBits(uint16_t value, bool bits[10]) {
    // Maskování posledních 8 bitů
    uint16_t last10Bits = value & 0x03FF;
    bool lastBit = 0;
    uint8_t strike = 0;

    // Extrahování jednotlivých bitů
    for (int i = 0; i < 10; i++) {
        bits[i] = (last10Bits >> i) & 0x01; // Získání konkrétního bitu

        if (bits[i] == lastBit) {
            strike++;
        } else {
            strike = 0;
        }

        if (strike >= 2) {
            bits[i] = !bits[i];
            strike = 0;
        }

        lastBit = bits[i];
    }
}

void render_frame(char *line1, char *line2) {
    LCD_Clear();

    LCD_String(line1);
    LCD_SetCursor(1, 0);
    LCD_String(line2);
}

void game() {
    LCD_Clear();
    // LCD_SetCursor(0, 1);
    // LCD_Char(3);
    // LCD_Char(4);

    bool game = 1;
    uint16_t score = 0;
    uint8_t multiplier = 2;
    bool lane = 0;
    char lane1[16];
    char lane2[16];

    char button;
    uint8_t btn_state[2];
    bool layout1[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0};
    bool layout2[] = {1, 0, 1, 0, 1, 1, 0, 0, 1, 0};

    int layout = 1;
    int cursor = 0;
    int trigger = 4;
    int box1 = -1;
    int box2 = -1;

    const char boxchar = '<';
    rst_milis();
    uint16_t delta_time; // = milis();
    // const char car[2] = {3, 4};
    // const char clean[2] = {' ', ' '};
    for (int i = 0; i <= 15; i++) {

        lane1[i] = ' ';
        lane2[i] = ' ';
    }

    while (game) {
        // if (milis() - scantime >= 10) {
        //   scantime = milis();
        button = LCD_Button_Get();
        if (button == 'u' && btn_state[0] < 101) {
            btn_state[0]++;
        } else if (button == 'd' && btn_state[1] < 101) {
            btn_state[1]++;
        } else {
            for (int x = 0; x <= 1; x++) {
                if (btn_state[x] > 0) {
                    btn_state[x]--;
                }
            }
        }
        //}
        if (milis() - delta_time >= 150 - (multiplier * 15)) {
            delta_time = milis();

            if (btn_state[0] > 50) {
                lane = 0;
            } else if (btn_state[1] > 50) {
                lane = 1;
            }

            game_tick(lane, lane1, lane2);

            score += multiplier;

            if (box1 >= 0) {
                if (box1 != 15) {
                    lane1[box1 + 1] = ' ';
                }
                lane1[box1] = boxchar;
                box1--;
            } else {
                lane1[0] = ' ';
                // box1 = 15;
            }

            if (box2 >= 0) {
                if (box2 != 15) {
                    lane2[box2 + 1] = ' ';
                }
                lane2[box2] = boxchar;
                box2--;
            } else {
                lane2[0] = ' ';
                // box2 = 15;
            }

            lane1[0] = ' ';
            lane2[0] = ' ';

            if (layout == 1) {
                if (layout1[cursor] && box2 == -1 && trigger > box1) {

                    box2 = 15;
                    cursor++;

                } else if (!layout1[cursor] && box1 == -1 && trigger > box2) {

                    box1 = 15;
                    cursor++;
                }
            }

            if (layout == 2) {
                if (layout2[cursor] && box2 == -1 && trigger > box1) {

                    box2 = 15;
                    cursor++;

                } else if (!layout2[cursor] && box1 == -1 && trigger > box2) {

                    box1 = 15;
                    cursor++;
                }
            }

            if (cursor >= 9) {
                cursor = 0;
                if (multiplier < 10) {
                    multiplier++;
                }
                if (trigger <= 8) {
                    trigger += 2;
                }
                if (layout == 1) {
                    layout = 2;
                } else {
                    layout = 1;
                }

                if (multiplier > 1) {
                    extractBits(milis() + btn_state[0], layout1);
                    extractBits(milis() + btn_state[1], layout2);
                }
            }

            render_frame(lane1, lane2);

            if ((box1 == 1 || box1 == 2) && !lane) {

                game = 0;
            } else if ((box2 == 1 || box2 == 2) && lane) {

                game = 0;
            }
        }
        delay_us(255);
    }

    game_over(score);
}

void main_menu() {
    bool select = 0;
    char btn = 'n';
    // char S[2];
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("LCD-Racer");
    LCD_SetCursor(0, 13);
    LCD_Char(0);
    LCD_Char(1);
    LCD_Char(2);

    LCD_SetCursor(1, 1);
    LCD_String("Game");

    LCD_SetCursor(1, 8);
    LCD_String("Credit");

    delay_ms(500);
    while (true) {
        delay_ms(50);

        if (select == 0) {
            LCD_SetCursor(1, 0);
            LCD_Char('>');
            LCD_SetCursor(1, 7);
            LCD_Char(' ');
        }

        else if (select == 1) {
            LCD_SetCursor(1, 0);
            LCD_Char(' ');
            LCD_SetCursor(1, 7);
            LCD_Char('>');
        }
        btn = LCD_Button_Get();
        if (btn == 'r') {
            select = 1;
        } else if (btn == 'l') {
            select = 0;
        } else if (btn == 's') {
            if (select == 0) {
                game();
                break;
            } else if (select == 1) {
                credits();
                break;
            }
        }
        /*
        LCD_SetCursor(1, 0);
        sprintf(S, "%u", select);
        LCD_String(S);
        */
    }
}

int main(void) {
    uint32_t time = 0;
    LCD_Init();
    LCD_Button_Init();

    /*UART3_Init(115200, UART3_WORDLENGTH_8D, UART3_STOPBITS_1, UART3_PARITY_NO,
               UART3_MODE_TX_ENABLE);
    UART3_Cmd(ENABLE);
    */
    LCD_CreateCustomChar(0, logoA);
    LCD_CreateCustomChar(1, logoB);
    LCD_CreateCustomChar(2, logoC);

    LCD_CreateCustomChar(3, carA);
    LCD_CreateCustomChar(4, carB);
    // Zobrazení textu na LCD
    init();

    while (1) {
        main_menu();
    }
    /*
        while (true) {
            uint16_t X = ADC_get(ADC2_CHANNEL_5);
            char S[16];

            sprintf(S, "%u", X);
            LCD_SetCursor(0, 0);
            LCD_String(S);

            LCD_SetCursor(1, 6);
            LCD_Char(LCD_Button_Get());
            delay_ms(100);
        }
        */
}
/*-------------------------------  Assert -----------------------------------*/
// #include "__assert__.h"
