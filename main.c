// Standard includes
#include <stdio.h>
#include <stdbool.h>

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
#include "esp32/esp32_com.h"

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


volatile bool esp32_connected = false;
volatile bool g_timeout_reached = true;

uint16_t color1 = 0x07E0; // GREEN
uint16_t color2 = 0xFD20; // ORANGE
uint16_t color3 = 0x8010; // PURPLE
uint8_t num_bins = 16;
uint8_t gravity_shift = 4;



static const uint16_t color_palette[] = {
     RED, GREEN, BLUE,
     CYAN, MAGENTA, YELLOW,
     ORANGE, PINK, PURPLE,
     LIME, NAVY, TEAL,
     WHITE, GREY, BLACK
 };

#define NUM_COLORS (sizeof(color_palette) / sizeof(color_palette[0]))


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




uint16_t GetNextColor(void) {
    static int color_index = 0; // Remembers its value between calls

    uint16_t selected_color = color_palette[color_index];

    // Move to the next index, or wrap back to 0 if at the end
    color_index = (color_index + 1) % NUM_COLORS;

    return selected_color;
}


static void InitSPI(void) {

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


static void InitUart(){

    while(!PRCMPeripheralStatusGet(PRCM_UARTA1)) {
            // Wait for clock/reset to stabilize
        }
    MAP_UARTConfigSetExpClk(UARTA1_BASE, 80000000,
                          UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));
}


void TimerTimeoutHandler(void) {
    // Clear the interrupt flag so it doesn't fire again immediately
    MAP_TimerIntClear(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);

    // Set our software flag
    g_timeout_reached = true;

    // Disable the timer so it doesn't keep running
    MAP_TimerDisable(TIMERA0_BASE, TIMER_A);

    Report("TIMER HANDLER \n");
}


void StartTimeoutTimer(unsigned long msecs) {
    // Enable the peripheral clock
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);

    // Configure as a one-shot 32-bit timer
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_ONE_SHOT);

    // Load the 2-second value (80Mhz * seconds)
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 80000000 * (msecs / 1000));

    // Register the interrupt handler
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerTimeoutHandler);

    // Enable the timeout interrupt
    MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);

    // Start the timer
    MAP_TimerEnable(TIMERA0_BASE, TIMER_A);

    Report("Started Timer \n");
}


void FormatAWSMessage(char *dest, int size, uint8_t bars, uint16_t c1, uint16_t c2, uint16_t c3, uint8_t grav, uint16_t rate) {

    // snprintf ensures we don't exceed the 'size' of the destination buffer
    // %u is for unsigned int, %04X prints hex with 4 digits (e.g., 0x07E0)
    snprintf(dest, size, "SEND_AWS <%u, 0x%04X, 0x%04X, 0x%04X, %u, %u>\n",
             bars, c1, c2, c3, grav, rate);
}

void
ChangeMode(char c ) {
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
        if (esp32_connected){

           Report("AWS Data Processing\n");
           const char *pMsg = "GET_AWS\n";
           const char *t;

           for (t = pMsg; *t != '\0'; t++) {
                      Uart1PutChar(*t);
              }

           char GET_buffer[512];

           // Copy current colors
           uint16_t c1 = color1 ; // GREEN
           uint16_t c2 = color2; // ORANGE
           uint16_t c3 = color3; // PURPLE

           uint16_t rate = 0;

           CC3200_Data aws_data = { num_bins, c1, c2, c3, gravity_shift, rate };
           g_timeout_reached = false;
           StartTimeoutTimer(10000);
           while(!g_timeout_reached) {

               // Check uART
               if (MAP_UARTCharsAvail(UART1BASE)) {
                    Report("."); // Heartbeat to show UART is alive
               }


               if (FetchInputNonBlocking(GET_buffer)) {
                   Report("Raw String Received: [%s]\n", GET_buffer);

                   if (ProcessIncomingData(GET_buffer, &aws_data) == 0){
                       Report("AWS Data received\n");
                         color1 = aws_data.c1;
                         color2 = aws_data.c2;
                         color3 = aws_data.c3;
                         num_bins = aws_data.bars;
                         gravity_shift = aws_data.grav;



                         // Full reset of samples
                         memset(g_ping, 0, sizeof(g_ping));
                         memset(g_pong, 0, sizeof(g_pong));
                         StartADCSampling(g_ping, g_pong, WINDOW_SIZE);

                       MAP_TimerDisable(TIMERA0_BASE, TIMER_A);
                       break;
                   } else {
                       Report("Invalid Data received\n");
                   }
               }
           }
        }
       break;
    case '5':
        // POST request to save current configuration

        break;
    case '7':
            // POST request to save current configuration
            color1 = GetNextColor();
            break;
    case '8':
            // POST request to save current configuration
            color2 = GetNextColor();
            break;
    case '9':
          // POST request to save current configuration
            color3 = GetNextColor();
          break;
    case '-': {
        char SEND_buffer[512];
        FormatAWSMessage(SEND_buffer, 512,  num_bins, color1, color2, color3, gravity_shift, 400);
        const char *t;

        for (t = SEND_buffer; *t != '\0'; t++) {
                  Uart1PutChar(*t);
          }
        break;
    }

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



    InitSystick();

    // Initialize all the Fast Fourier Transform stuff
    InitFFT();

    // Set up SPI for communications with OLED
    InitSPI();

    MAP_UtilsDelay(80000000);
    // Start up Uart
    InitUart();


    fillScreen(BLACK);



     esp32_connected = false;

//    uint16_t rate = 0;
//    char rate[16];
//
//    if ( ProcessIncomingData(GET_buffer, &aws_data) == 0){
//                  color1 = aws_data.c1;
//                  color2 = aws_data.c2;
//                  color3 = aws_data.c3;

  

//    const char *pMsg = "GET_AWS\n";
    const char *pMsg = "STATUS\n";
    const char *t;

    char GET_buffer[512];

    for (t = pMsg; *t != '\0'; t++) {
            Uart1PutChar(*t);
//        Report("CHARACTER: %c ", *t);
    }
//    Uart1PutChar('\0');
    Message("Status: sent message to ESP32...\n\r");

    int timeout_count = 0;
    int max_timeout = 30000;


    while(timeout_count < max_timeout) {
//        MAP_UtilsDelay(1000);
        if (FetchInputNonBlocking(GET_buffer)) {
            Report("Got something \n");
            // Once we have a string, parse it
            if ( CheckStatus(GET_buffer) == 0){
                ;
                esp32_connected = true;
                Report("ESP 32 connection checked and verified \n");
                break; // End startup loop
            } else{
                Report("Unable to verify validity of esp32");
            }
        }
        MAP_UtilsDelay(8000 / 3);
        timeout_count++;

        if (timeout_count >= max_timeout) {
            Report("TIMEOUT: ESP32 not responding. Using defaults.\n\r");
        }
    }

    q15_t bin_peaks[MAX_POSSIBLE_BARS] = {0}; // we will only use up to `num_bars` though



    fillScreen(BLACK);

    SetupADCMic(ADC_SAMPLE_RATE);
    StartADCSampling(g_ping, g_pong, WINDOW_SIZE);


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
