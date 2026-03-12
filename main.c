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
#include "oled/oled.h"
#include "mic/adc_mic.h"

#include <stdbool.h>


static q15_t g_ping[WINDOW_SIZE];
static q15_t g_pong[WINDOW_SIZE];
//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************
//#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
//#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

q15_t frequency_magnitudes[FFT_SIZE/2];  // The final, usable volume levels for the display
q15_t audio_inputs[FFT_SIZE];            // Raw ADC microphone readings

//volatile uint16_t sample_index = 0;   // Keeps track of where we are in the audio inputs array for timer interrupts
//volatile uint8_t frame_ready = false;     // The flag
mode_t mode = BAR;

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void BoardInit(void);


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
InitSPI(void) {

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,
                               MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                               SPI_IF_BIT_RATE,
                               SPI_MODE_MASTER,
                               SPI_SUB_MODE_0,
                               (SPI_SW_CTRL_CS |
                                SPI_4PIN_MODE |
                                SPI_TURBO_OFF |
                                SPI_CS_ACTIVELOW |
                                SPI_WL_8)
                               );

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    Adafruit_Init();
}

void
ChangeMode(char c) {
    switch (c) {
    case '1':
        mode = BAR;
        fillScreen(BLACK);
        Report("Mode is now BAR\n\r");
        break;
    case '2':
        mode = WAVE;
        fillScreen(BLACK);
        Report("Mode is now WAVE\n\r");
        break;
    case '3':
        mode = PULSE;
        fillScreen(BLACK);
        Report("Mode is now PULSE\n\r");
        break;
    case '4':
        // GET request to load configuration
        break;
    case '5':
        // POST request to save current configuration
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

    PRCMCC3200MCUInit();
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
//#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
//#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);
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

    SetupADCMic(ADC_SAMPLE_RATE);

    InitSystick();

    // Initialize all the Fast Fourier Transform stuff
    InitFFT();

    // Set up SPI for communications with OLED
    InitSPI();

    fillScreen(BLACK);

    StartADCSampling(g_ping, g_pong, WINDOW_SIZE);

    uint8_t num_bins = 16;
    uint8_t gravity_shift = 4;
    uint16_t color1 = 0x07E0; // GREEN
    uint16_t color2 = 0xFD20; // ORANGE
    uint16_t color3 = 0x8010; // PURPLE

    // Set up the bars and peaks here before the loop
    q15_t bin_peaks[MAX_POSSIBLE_BARS] = {0}; // we will only use up to `num_bars` though

    fillScreen(BLACK);


    while(1)
    {
        ButtonPress(ChangeMode);

        int readyBuffer = CheckBufferReady();
        if (readyBuffer == BUFFER_PING) {
            ProcessAudioFrame(g_ping, frequency_magnitudes, gravity_shift);
            BinPeaks(frequency_magnitudes, num_bins, bin_peaks);

            DrawVisuals(mode, num_bins, bin_peaks, color1, color2, color3);
            ClearBufferFlag(BUFFER_PING);
        }
        else if (readyBuffer == BUFFER_PONG) {
            ProcessAudioFrame(g_pong, frequency_magnitudes, gravity_shift);
            BinPeaks(frequency_magnitudes, num_bins, bin_peaks);

            DrawVisuals(mode, num_bins, bin_peaks, color1, color2, color3);
            ClearBufferFlag(BUFFER_PONG);
        }
        else if (readyBuffer == -1){
            ClearOverrunFlag();
        }

    }
}

//    q15_t sample_peaks[16] = {
//        120, 115, 90, 60,  // Deep Bass (Bars 0-3)
//        40,  30,  25, 20,  // Low Mids (Bars 4-7)
//        15,  12,  10, 10,  // High Mids (Bars 8-11)
//        8,   5,   5,  10   // Treble (Bars 12-15)
//    };
