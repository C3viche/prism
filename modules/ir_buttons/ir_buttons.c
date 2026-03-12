#include "ir_buttons.h"

static volatile int systick_expired = 0;
static volatile int pulse_num = 0;
static volatile uint64_t ulsystick_delta_us = 0;
static volatile uint64_t ulsystick_deltas[100];

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

        SysTickReset();
    }

    return;
}

static void
SysTickHandler(void) {
    systick_expired = 1;
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

static char
Decode(int mask) {

    char c;
    // NUMBER 1: 0x4202
    if (mask == 0x4202) {
        c = '1';
    }
    // NUMBER 2: 0x4102
    else if (mask == 0x4102) {
        c = '2';
    }
    // NUMBER 3: 0x4302
    else if (mask == 0x4302) {
        c = '3';
    }
    else if (mask == 0x4082) {
        c = '4';
    }
    else if (mask == 0x4282) {
        c = '5';
    }
    else if (mask == 0x4382){
        c ='7';
    }
    else if (mask == 0x4042){
        c ='8';
    }
    else if (mask == 0x4242){
        c ='9';
    }
    else if (mask == 0x43A2){
        c ='-';
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

void
ButtonPress(void (*f) (char)) {
    if (systick_expired && pulse_num > 0 ){
        int mask = ReadCode();

        // Decode the mask into a readable character just to check if garbage
        char c = Decode(mask);

        // Call the handler on the button press
        f(c);

        // Reset
        memset((void*)ulsystick_deltas, 0, sizeof(ulsystick_deltas)); // fill first 15 with zeroes
        systick_expired = 0;
    }

}
