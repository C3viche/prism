/*
 * test.h
 *
 *  Created on: Jan 27, 2024
 *      Author: rtsang
 */

#ifndef OLED_OLED_TEST_H_
#define OLED_OLED_TEST_H_

#include <math.h> // Required for sinf()
#include <stdbool.h>

#include "glcdfont.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"

#include "prism_utils.h"
#include "arm_math.h"
#include "dsp/fast_math_functions.h"

#include "uart_if.h"

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

#define BASS_COLOR GREEN
#define MID_COLOR ORANGE
#define TREBLE_COLOR YELLOW

// Math Constants
#define TWO_PI 6.28318f
#define PHASE_TRAVEL 0.5f

// Useful color macros
#define EXTRACT_RED(color)   ((((color) >> 11) & 0x1F) << 3)
#define EXTRACT_GREEN(color) ((((color) >> 5) & 0x3F) << 2)
#define EXTRACT_BLUE(color)  (((color) & 0x1F) << 3)

// Fades a color intensity to 0 based on Y position
#define FADE_TO_BLACK(base, y, max_y) (((base) * ((max_y) - (y))) / (max_y))

// Blends a base color intensity into a target color intensity based on Y position
#define BLEND_TO_TARGET(base, target, y, max_y) ((((base) * ((max_y) - (y))) + ((target) * (y))) / (max_y))

// Custom functions
void drawFontTable(int x, int y, unsigned int color, unsigned int bg, unsigned char size);
void drawString(int x, int y, char *str, unsigned int color, unsigned int bg, unsigned char size);
void drawHorizontalLines(unsigned int color);
void drawVerticalLines(unsigned int color);

void DrawVisuals(mode_t mode, size_t num_bins, q15_t* bin_peaks);

#endif /* OLED_OLED_TEST_H_ */
