#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

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
    int MAX_ESP_LENGTH = 512;

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

int ProcessIncomingData(char *msg) {
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
//            if (strlen(segments[0]) > 0) UpdateBars(segments[0]);
//            if (strlen(segments[1]) > 0) UpdateColor1(segments[1]);
//            if (strlen(segments[2]) > 0) UpdateColor2(segments[2]);
//            if (strlen(segments[3]) > 0) UpdateColor3(segments[3]);
//            if (strlen(segments[4]) > 0) UpdateGravity(segments[4]);
//            if (strlen(segments[5]) > 0) UpdateRate(segments[5]);
            char statusReport[128] = "Update Summary: ";

            // Process each and append a quick code to the report string
            // [B]=Bars, [C]=Colors, [G]=Gravity, [R]=Rate
            // An uppercase letter means Updated, lowercase or dot means Skipped

            if (strlen(segments[0]) > 0) {      strcat(statusReport, segments[0]); }
            if (strlen(segments[1]) > 0) {    strcat(statusReport, segments[1]); }
            if (strlen(segments[2]) > 0) {    strcat(statusReport, segments[2]); }
            if (strlen(segments[3]) > 0) {   strcat(statusReport, segments[3]); }
            if (strlen(segments[4]) > 0) {   strcat(statusReport, segments[4]); }
            if (strlen(segments[5]) > 0) {     strcat(statusReport, segments[5]); }

            // Final consolidated report
            Report("\n\r%s - OK\n\r", statusReport);
            Report("Parsing Complete. Non-blank fields updated.\n\r");
            return 0;
        }
    }
    return -1;
}
