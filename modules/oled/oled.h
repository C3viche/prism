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
// Standard Colors (RGB565)
#define BLACK           0x0000
#define WHITE           0xFFFF
#define GREY            0x8410

// Primary Colors
#define RED             0xF800
#define GREEN           0x07E0
#define BLUE            0x001F

// Secondary Colors
#define CYAN            0x07FF  // Green + Blue
#define MAGENTA         0xF81F  // Red + Blue
#define YELLOW          0xFFE0  // Red + Green

// Fun Colors
#define ORANGE          0xFD20
#define PINK            0xF819
#define PURPLE          0x8010
#define LIME            0x07FF
#define NAVY            0x000F
#define TEAL            0x0410


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
