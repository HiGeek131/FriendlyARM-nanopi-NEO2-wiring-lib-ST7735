#ifndef __ST7735_H
#define __ST7735_H

#include "wiringPi.h"
#include "wiringPiSPI.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define RED  	0x00f8
#define GREEN	0xe007
#define BLUE 	0x1f00
#define WHITE	0xffff
#define BLACK	0x0000
#define YELLOW  0xE0FF
#define GRAY0   0x7DEF
#define GRAY1   0x1084
#define GRAY2   0x0842

#define ST7735_DC 5
#define ST7735_RST 6
#define ST7735_BLK 18

#define SPI_CHANNEL 0
#define SPI_SPEED 50000000

#define LCD_HORIZONTAL  0xa8
#define LCD_VERTICAL    0xc8

#define X_MAX_PIXEL	        160
#define Y_MAX_PIXEL	        80

typedef struct
{
    int channel;
    int speed;
    int displayMode;
    uint16_t displayBuffer[12800];
} ST7735_TypeDef;

extern ST7735_TypeDef st7735_cfg;

int LCD_Init(ST7735_TypeDef* st7735_cfg);
void LCD_WriteCom(uint8_t Com);
void LCD_ClearBuffer(uint16_t Color);
void LCD_ClearScreen(uint16_t Color);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t Data);
void LCD_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end);
void LCD_DisplayImage(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end, uint16_t *image, uint16_t len);
void LCD_DisplayBuffer(void);
void LCD_ConvertASCIIToBuffer(uint8_t line, uint8_t* p, uint16_t wordColor, uint16_t backColor);

#endif
