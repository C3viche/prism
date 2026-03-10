/*
 * texting_client.h
 *
 *  Created on: Feb 24, 2026
 *      Author: crisemble
 */

#ifndef TEXTING_CLIENT_H_
#define TEXTING_CLIENT_H_

// Standard includes
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_nvic.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "uart_if.h"
#include "math.h"

// OLED includes
// Driverlib includes
#include "spi.h"
#include "uart.h"
#include "i2c_if.h"

// Common interface includes
//#include "Adafruit_SSD1351.h"
//#include "oled_test.h"

// Systick and timer stuff
#include "gpio_if.h"
#include "systick.h"

#include "timer.h"
#include "prcm.h"

//#include "pin_mux_config.h"

#define MICROSECONDS_TO_TICKS(us)   ((SYS_CLK/1000000) * (us))
// some helpful macros for systick

#define SYSCLKFREQ 80000000ULL
#define TICKS_TO_US(ticks) \
        ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
        ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

// pin 50 info
#define IR_GPIO_PORT GPIOA0_BASE
#define IR_GPIO_PIN 0x1

// Useful core logic macros
#define MAX_PULSES 1000
#define MAX_STRINGS 32767 // 111111111111111 in hex
#define CODE_LEN 15
#define CURSOR_TICKS 120000000

// OLED STUFF //
#define FRAME_RATE_DELAY 600000

#define SPI_IF_BIT_RATE  10000000
#define TR_BUFF_SIZE     100

#define ALEX_OLED        UARTA0_BASE
#define ALEX_PERIPH      PRCM_UARTA0
#define UartGetChar()               MAP_UARTCharGet(ALEX_OLED)
#define UartPutChar(c)              MAP_UARTCharPut(ALEX_OLED,c)
#define UartCharsAvail()            MAP_UARTCharsAvail(ALEX_OLED)
#define UartCharGetNonBlocking()    MAP_UARTCharGetNonBlocking(ALEX_OLED)
#define UartCharPutNonBlocking(c)   MAP_UARTCharPutNonBlocking(ALEX_OLED,c)

#define MAX_STRING_LENGTH 100

// From i2c_if.c
#define FAILURE                 -1
#define SUCCESS                 0
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

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

#define LED_COUNT 3

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended


/* Exposed Helper Functions */
void InitSystick(void);
void ButtonPress(void (*f) (char));

#endif /* TEXTING_CLIENT_H_ */
