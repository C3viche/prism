#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "prcm.h"
#include "interrupt.h"
#include "pin_mux_config.h"
#include "uart_if.h"
#include "uart.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"


#include "mic/adc_mic.h"
#include "esp32/esp32_com.h"


#include "uart_if.h"

#define CONSOLE              UARTA0_BASE
#define UART1BASE                UARTA1_BASE
#define UART1_PERIPH             PRCM_UARTA1
#define UartGetChar()       MAP_UARTCharGet(CONSOLE)
#define UartPutChar(c)      MAP_UARTCharPut(CONSOLE,c)
#define Uart1GetChar()      MAP_UARTCharGet(UART1BASE)
#define Uart1PutChar(c)     MAP_UARTCharPut(UART1BASE,c)


#define UART_BUFFER_SIZE 100

#define UartCharsAvail()    MAP_UARTCharsAvail(UART1BASE)
#define UartCharGetNonBlocking()    MAP_UARTCharGetNonBlocking(UART1BASE)
#define UartCharPutNonBlocking(c)   MAP_UARTCharPutNonBlocking(UART1BASE,c)


// Baud rate should match your ESP32-C5 (115200)
#define UART_BAUD_RATE 115200
#define SYS_CLK 80000000


 int FetchInput(char * finalMsg){
    char g_rxBuffer[512];

    int rxIndex = 0;
    char c;
//    drawString(10, 60, "GOT MESSAGE!", WHITE, BLACK, 1);
    while (1) {
        c = Uart1GetChar();

        if (c == '\0' || c == '\r' || c == '\n') {
            if (rxIndex > 0) {
                g_rxBuffer[rxIndex] = '\0';
                strcpy(finalMsg, g_rxBuffer);
                rxIndex = 0; // Reset for next time
                return 1;    // Exit function with success
            }
            continue;
        }

        if (rxIndex < MAX_ESP_LENGTH - 1) {
                    g_rxBuffer[rxIndex] = c;
                    rxIndex++;
        } else {
            g_rxBuffer[rxIndex] = '\0';
            strcpy(finalMsg, g_rxBuffer);
            rxIndex = 0;
            return 1;
        }
    }


}

int ProcessIncomingData(char *msg, CC3200_Data *data) {
    // Check for our specific header
    if (strncmp(msg, "DATA<", 5) == 0) {
        char *ptr = msg + 5; // Start after 'DATA<'

        // Find and remove the closing bracket
        char *endBracket = strchr(ptr, '>');
        if (endBracket) *endBracket = '\0';

        char *segments[6];
        int i = 0;
        char *current = ptr;
        char *next;

        // Manual slicing to handle empty fields like ",,"
        while (current != NULL && i < 6) {
            next = strchr(current, ',');
            if (next != NULL) {
                *next = '\0'; // Split the string
            }

            segments[i] = current;

            if (next != NULL) {
                current = next + 1;
            } else {
                current = NULL;
            }
            i++;
        }

        // Apply only non-blank values to your system
        if (i == 6) {
            char statusReport[128] = "Update Summary: ";

            // BARS (String)
            if (strlen(segments[0]) > 0) {
                strncpy(data->bars, segments[0], 15);
                data->bars[15] = '\0'; // Safety null terminator
                strcat(statusReport, "Bars ");
            }

            // COLORS (Hex to Uint16)
            if (strlen(segments[1]) > 0) {
                data->c1 = (uint16_t)strtoul(segments[1], NULL, 16);
                strcat(statusReport, "C1 ");
            }
            if (strlen(segments[2]) > 0) {
                data->c2 = (uint16_t)strtoul(segments[2], NULL, 16);
                strcat(statusReport, "C2 ");
            }
            if (strlen(segments[3]) > 0) {
                data->c3 = (uint16_t)strtoul(segments[3], NULL, 16);
                strcat(statusReport, "C3 ");
            }

            // GRAVITY (String)
            if (strlen(segments[4]) > 0) {
                strncpy(data->grav, segments[4], 15);
                data->grav[15] = '\0';
                strcat(statusReport, "Grav ");
            }

            // RATE (String)
            if (strlen(segments[5]) > 0) {
//                strncpy(data->rate, segments[5], 15);
                data->rate = (uint16_t) 12;
            }

            // Final consolidated report
            Report("\n\r%s - OK\n\r", statusReport);
            Report("Parsed Hex Colors: C1:0x%04X, C2:0x%04X, C3:0x%04X\n\r", data->c1, data->c2, data->c3);
            return 0;
        }
    }
    return -1;
}
