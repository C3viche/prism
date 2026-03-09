#include "texting_client.h"

static volatile int systick_expired = 0;
static volatile int pulse_num = 0;
static volatile uint64_t ulsystick_delta_us = 0;
static volatile bool gpio_prev_state = 0;
static volatile uint64_t ulsystick_deltas[100];
static volatile bool cursor_timeout = false;

//*****************************************************************************
//
//! Interrupt Handlers
//
//*****************************************************************************
static inline void
SysTickReset(void) {
    HWREG(NVIC_ST_CURRENT) = 1;
    systick_expired = 0;
}

static void
GPIOIntHandler(void) {

    // get and clear status
    unsigned long ulStatus;
    ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, true);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus);

    // check in interrupt occured on pin 50
    if (ulStatus & IR_GPIO_PIN) {

        unsigned char pinVal = MAP_GPIOPinRead(IR_GPIO_PORT, IR_GPIO_PIN);

        ulsystick_delta_us = TICKS_TO_US(SYSTICK_RELOAD_VAL - MAP_SysTickValueGet());
        SysTickReset();

        // If systick expired, we timed out (end of transmission or first pulse)
        // Reset pulse count if we timed out
        if (systick_expired) {
            pulse_num = 0;
            systick_expired = 0;
            // Don't record this delta as it includes idle time

        } else {
            // Only record delta if part of remote code. Remove noise
            bool within_range = (ulsystick_delta_us > 500 && ulsystick_delta_us < 1000) ||
                                (ulsystick_delta_us > 1500 && ulsystick_delta_us < 2000);
            if (within_range)
            {
                ulsystick_deltas[pulse_num] = ulsystick_delta_us;
                pulse_num++;
            }
        }
    }

    return;
}

static void
SysTickHandler(void) {
    systick_expired = 1;
}

static void
CursorTimerIntHandler(void) {
    // Clear the interrupt flag so it doesn't fire again immediately
    unsigned long ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);

    // Signal main loop that time is up
    cursor_timeout = true;
}


//*****************************************************************************
//
//! Helper Functions
//
//*****************************************************************************

void
InitSystick(void) {
    // Set up systick configuration and handler
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);
    MAP_SysTickIntRegister(SysTickHandler);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();

    // Register the interrupt handlers
    MAP_GPIOIntRegister(IR_GPIO_PORT, GPIOIntHandler);

    // Configure interrupts on rising and falling edges
    MAP_GPIOIntTypeSet(IR_GPIO_PORT, IR_GPIO_PIN, GPIO_BOTH_EDGES); // read ir_output
    uint64_t ulStatus = MAP_GPIOIntStatus(IR_GPIO_PORT, false);
    MAP_GPIOIntClear(IR_GPIO_PORT, ulStatus); // clear interrupts on GPIOA0

    // Actually enable interrupts on ir_output now
    MAP_GPIOIntEnable(IR_GPIO_PORT, IR_GPIO_PIN);
}

void
InitCursorTimer(void) {
    // Enable the clock for Timer A0
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);

    // Configure as a ONE-SHOT timer (counts down once, then stops)
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_ONE_SHOT);

    // 3. Set timeout to ~1.5 seconds (80MHz clock)
    // 80,000,000 ticks = 1 second. 120,000,000 = 1.5 seconds (roughly)
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, CURSOR_TICKS);

    // Register the interrupt and enable for timeout
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, CursorTimerIntHandler);
    MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
}

static void
ResetCursorTimer(void) {
    // Stop current timer and reload value (1.5 seconds)
    MAP_TimerDisable(TIMERA0_BASE, TIMER_A);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, CURSOR_TICKS);

    // Restart timer
    MAP_TimerEnable(TIMERA0_BASE, TIMER_A);
}

static char
Decode(int mask, int char_idx) {

    char c;
    // NUMBER 1: 0x4202
    if (mask == 0x4202) {
        c = '1';
    }
    // NUMBER 2: 0x4102
    else if (mask == 0x4102) {
        char chars[3] = {'A', 'B', 'C'};
        c = chars[char_idx % 3];
    }
    // NUMBER 3: 0x4302
    else if (mask == 0x4302) {
        char chars[3] = {'D', 'E', 'F'};
        c = chars[char_idx % 3];
    }
    // NUMBER 4: 0x4082
    else if (mask == 0x4082) {
        char chars[3] = {'G', 'H', 'I'};
        c = chars[char_idx % 3];
    }
    // NUMBER 5: 0x4282
    else if (mask == 0x4282) {
        char chars[3] = {'J', 'K', 'L'};
        c = chars[char_idx % 3];
    }
    // NUMBER 6: 0x4182
    else if (mask == 0x4182) {
        char chars[3] = {'M', 'N', 'O'};
        c = chars[char_idx % 3];
    }
    // NUMBER 7: 0x4382
    else if (mask == 0x4382) {
        char chars[4] = {'P', 'Q', 'R', 'S'};
        c = chars[char_idx % 4];
    }
    // NUMBER 8: 0x4042
    else if (mask == 0x4042) {
        char chars[3] = {'T', 'U', 'V'};
        c = chars[char_idx % 3];
    }
    // NUMBER 9: 0x4242
    else if (mask == 0x4242) {
        char chars[4] = {'W', 'X', 'Y', 'Z'};
        c = chars[char_idx % 4];
    }
    // NUMBER 0: 0x4142
    else if (mask == 0x4142) {
        c = ' ';
    }
    // MUTE: 0x43a2
    else if (mask == 0x43a2) {
        c = '{';
    }
    // LAST: 43d2
    else if (mask == 0x43d2) {
        c = '}';
    }
    else {
        c = '?'; // No pair matched
    }

    return c;
}

static int
ReadCode(void) {
    int mask = 0;

    int i = 0;
    for(i = 0; i < CODE_LEN; i++) {

        // Threshold is around 1800 but I'm doing 1500 just to be safe
        if (ulsystick_deltas[i] < 1500) {
            mask = (mask << 1) + 0; // short pulse
        } else {
            mask = (mask << 1) + 1; // long pulse
        }
    }

    pulse_num = 0; // Reset back to start
    return mask;
}

// Initialize the stuff for UART1 to use it
static void
InitUART()
{
  MAP_UARTConfigSetExpClk(ALEX_OLED,MAP_PRCMPeripheralClockGet(ALEX_PERIPH),
                  UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));
}

static char g_rxBuffer[MAX_STRING_LENGTH + 1];

// Returns 1 if a NEW message has arrived (and copies it to 'finalMsg'), 0 otherwise
int
FetchInput(char *finalMsg)
{
    static int rxIndex = 0; // initialized once at startup
    int msgReceived = 0;

    // Loop while there are characters in the hardware queue
    while (UartCharsAvail()) {
        char c = UartCharGetNonBlocking();

        // Check for "Enter" key (Null character)
        if (c == '\0' || c == '\r' || c == '\n') {
            g_rxBuffer[rxIndex] = '\0';       // Terminate string
            strcpy(finalMsg, g_rxBuffer);     // Copy to output
            rxIndex = 0;                      // Reset buffer for NEXT message
            msgReceived = 1;                  // Flag success
            break;
        }
        // Otherwise, keep building the string
        else if (rxIndex < MAX_STRING_LENGTH) {
            g_rxBuffer[rxIndex] = c;
            rxIndex++;
        }
    }

    return msgReceived;
}

static void
SendText(char msg[MAX_STRING_LENGTH+1]) {
    if (msg != NULL) {
        while (*msg != '\0') {
            UartPutChar(*msg);
            msg++;
        }

        // Send a terminator so the other side knows the message is done
        UartPutChar('\0');
    }
}

void
ButtonPress(int* lRetVal, int* prev_mask, int* char_idx, int* cursor, bool* enter, char* msg, int (*f) (int, char*)) {
    if (systick_expired && pulse_num > 0 ){
        int mask = ReadCode();

        // Decode the mask into a readable character just to check if garbage
        char check = Decode(mask, *char_idx);

        // Only process if we got around 15 pulses
        if (check != '?') {

            // Cycle through letters if same button is pressed
            if(mask == *prev_mask) {
                (*char_idx)++;
            } else {                        // This is the value for ENTER
                if (*prev_mask != 0 && *prev_mask != 0x43a2 && *prev_mask != 0x43d2) { (*cursor)++; } // Move cursor forward ONLY if this isn't a "new" button press
                *char_idx = 0;
            }

            // Now decode the actual letter to use
            char c = Decode(mask, *char_idx);

            // Add the character and report the updated message
            if (c == '{') { // MUTE == ENTER --> send message!
                *enter = true;
            } else if (c == '}') { // LAST
                if (*cursor > 0) { // don't go out of msg buffer
                    msg[*cursor] = '\0';
                    (*cursor)--;
                    msg[*cursor] = '\0';
                }
            } else {
                msg[*cursor] = c;
            }

            // Outgoing Text
            msg[*cursor + 1] = '\0';
    //        fillRect(35, 50, 100, 10, BLACK); // Re-clear text message box
    //        drawString(35, 50, msg, WHITE, BLACK, 1);
//            ClearTerm();

            if (*enter) {
//                int RetVal = -1;
                f(*lRetVal, msg); // handler is SendText in most cases
                *enter = false;
            }

            // Update previous mask and reset the cursor timer
            *prev_mask = mask;

            ResetCursorTimer();
        }

        // Reset
        memset((void*)ulsystick_deltas, 0, CODE_LEN * sizeof(uint64_t)); // fill first 15 with zeroes
        systick_expired = 0;
    }

}

void
CheckButtonTimeout(int* prev_mask, int* char_idx, int* cursor){
    if (cursor_timeout) {
        if (*prev_mask != 0x43a2) {
            (*cursor)++;
            if(*cursor >= MAX_STRING_LENGTH) *cursor = MAX_STRING_LENGTH-1;
        }
        *char_idx = 0;  // Reset letter cycling
        *prev_mask = 0; // Force next button to be a "new" press

        cursor_timeout = false; // Reset flag
    }
}

//****************************************************************************
//
//! Main function
//
//****************************************************************************
//int
//main()
//{
//    //
//    // Initialize Board configurations
//    //
//    BoardInit();
//
//    //
//    // Power on the corresponding GPIO port B for 9,10,11.
//    // Set up the GPIO lines to mode 0 (GPIO)
//    //
//    PinMuxConfig();
//
//    // Set up our two timers
//    InitSystick();
//    InitCursorTimer();
//
//    InitUART();
//
//    // I2C Init
//    I2C_IF_Open(I2C_MASTER_MODE_FST);
//
//    // Set up SPI for communications
//    InitSPI();
//
//    fillScreen(BLACK);
//
//    // State variables for texting
//    char incomingMsg[MAX_STRING_LENGTH + 1] = "Waiting...";
//    char msg[MAX_STRING_LENGTH + 1] = {0};
//
//    int cursor = 0;
//    int char_idx = 0;
//    int prev_mask = 0;
//    bool enter = false;
//
//    // Draw initial UI labels
////    drawString(10, 10, "FRIEND:", WHITE, BLACK, 1);
////    drawString(10, 50, "YOU:", WHITE, BLACK, 1);
//
//    printf("Waiting for pulses...\n");
//
//    while(1)
//    {
//        // Incoming Text
////        if (FetchInput(incomingMsg)) {
////            fillRect(60, 10, 100, 10, BLACK);
////            drawString(60, 10, incomingMsg, WHITE, BLACK, 1);
////        }
//
//        // Check the timeout for a valid wait
//        if (cursor_timeout) {
//            if (prev_mask != 0x43a2) {
//                cursor++;
//                if(cursor >= MAX_STRING_LENGTH) cursor = MAX_STRING_LENGTH-1;
//            }
//            char_idx = 0;  // Reset letter cycling
//            prev_mask = 0; // Force next button to be a "new" press
//
//            cursor_timeout = false; // Reset flag
//        }
//
//
//        // Wait for systick to expire and for there to be input before processing
//        if (systick_expired && pulse_num > 0 ){
//            ButtonPress(&prev_mask, &char_idx, &cursor, &enter, msg, SendText);
//        }
//    }
//
//    /*
//     * A 1 if long pulse and a 0 if short pulse
//     *
//     * var = (var << 1) + (1 or 0)
//     *
//     * final hex will represent the encoding of that button
//     *
//     * */
//
//
//    return 0;
//
//}
