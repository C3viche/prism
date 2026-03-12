/*
 * prism_utils.h
 *
 *  Created on: Mar 7, 2026
 *      Author: crisemble
 */

#ifndef MODULES_PRISM_UTILS_H_
#define MODULES_PRISM_UTILS_H_

#include <stdio.h>

// The CC3200’s Analog-to-Digital Converter (ADC) only understands positive voltages.
// It has a 12-bit resolution, meaning it translates voltage into a strict scale from 0 to 4095
#define MAX_ADC_VALUE ((1 << 12) - 1)
#define ADC_DC_OFFSET (1 << 11)

// BASS: 20 - 200 HZ
// MID:  200 - 2000 HZ
// TREBLE 2000 - 20000
// Spread for mid, bass, and treble ranges (for colors)
#define BASS 0.3f
#define MID 0.7f
#define TREBLE 1.0f

#define FFT_SIZE        512

#define WINDOW_SIZE 512

#define OLED_DIM   128

#define BUFFER_NONE  0
#define BUFFER_PING  1
#define BUFFER_PONG  2

#define MAX_MAGNITUDE   200

#define ADC_SAMPLE_RATE 16000

#define UART1BASE                UARTA1_BASE
#define UART1_PERIPH             PRCM_UARTA1
#define UART1BASE                UARTA1_BASE
#define UART1_PERIPH             PRCM_UARTA1
#define Uart1GetChar()      MAP_UARTCharGet(UART1BASE)
#define Uart1PutChar(c)     MAP_UARTCharPut(UART1BASE,c)

typedef enum modes {
    BAR,
    WAVE,
    PULSE
} mode_t;

#endif /* MODULES_PRISM_UTILS_H_ */

