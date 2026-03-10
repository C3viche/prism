/*
 * test.h
 *
 *  Created on: Jan 27, 2024
 *      Author: rtsang
 */

#ifndef OLED_OLED_TEST_H_
#define OLED_OLED_TEST_H_

#include "glcdfont.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"

#include "prism_utils.h"
#include "arm_math.h"

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF


void testfastlines(unsigned int color1, unsigned int color2);
void testdrawrects(unsigned int color);
void testfillrects(unsigned int color1, unsigned int color2);
void testfillcircles(unsigned char radius, unsigned int color);
void testdrawcircles(unsigned char radius, unsigned int color);
void testtriangles();
void testroundrects();
void testlines(unsigned int color);
void lcdTestPattern(void);

// Custom functions
void drawFontTable(int x, int y, unsigned int color, unsigned int bg, unsigned char size);
void drawString(int x, int y, char *str, unsigned int color, unsigned int bg, unsigned char size);
void drawHorizontalLines(unsigned int color);
void drawVerticalLines(unsigned int color);
void DrawBars(size_t num_bars, q15_t* bin_peaks, unsigned int color1, unsigned int color2, unsigned int color3);

#endif /* OLED_OLED_TEST_H_ */
