// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
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

// Common interface includes
#include "gpio_if.h"

#include "pin_mux_config.h"

#define APPLICATION_VERSION     "1.4.0"

// Custom Prism module includes
#include "fft/fft.h"
#include "binning/binning.h"
#include "ir_buttons/ir_buttons.h"
#include <stdbool.h>

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

q15_t frequency_magnitudes[FFT_SIZE/2];  // The final, usable volume levels for the display
q15_t audio_inputs[FFT_SIZE];            // Raw ADC microphone readings

volatile uint16_t sample_index = 0;   // Keeps track of where we are in the audio inputs array for timer interrupts
volatile uint8_t frame_ready = false;     // The flag
mode_t mode = BAR;

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void BoardInit(void);
static void ChangeMode(char c);


static void
DisplayBanner()
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t                       PRISM                      \n\r");
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

static void
ChangeMode(char c) {
    switch (c) {
    case '1':
        mode = BAR;
        Report("Mode is now BAR\n");
        break;
    case '2':
        mode = WAVE;
        Report("Mode is now WAVE\n");
        break;
    case '3':
        mode = PULSE;
        Report("Mode is now PULSE\n");
        break;
    default:
        break;
    }
}


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//                           MAIN FUNCTION
//****************************************************************************
int
main()
{
    // Initialize Board configurations
    BoardInit();

    // Power on the pinmux configurations
    PinMuxConfig();

    InitTerm();
    ClearTerm();

    // Display banner and usage message
    DisplayBanner();

    InitSystick();

    // Initialize all the Fast Fourier Transform stuff
    InitFFT();

    // Set up the bars and peaks here before the loop
    uint8_t num_bars = 3;
    q15_t bin_peaks[MAX_POSSIBLE_BARS] = {0}; // we will only use up to `num_bars` though

    while(1)
    {
        ButtonPress(ChangeMode);

        if (frame_ready) {

            // The buffer is full. Process the FFT_SIZE number of samples.
            // Populates `frequency_magnitudes` with scaled/processed magnitudes for different frequencies
            ProcessAudioFrame(audio_inputs, frequency_magnitudes);

            // Separate peaks into different bins and populate `bin_peaks`
            BinPeaks(frequency_magnitudes, num_bars, bin_peaks);

            // TODO: Update LED drawing here
            // DrawVisual(mode, bin_peaks)


            // Reset the index and lower the flag so the interrupt starts filling it again
            sample_index = 0;
            frame_ready = false;
        }

    }
}
