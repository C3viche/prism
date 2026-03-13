#include "mic/adc_mic.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_adc.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "adc.h"
#include "prcm.h"
#include "rom_map.h"
#include "uart_if.h" // For Report()
#include "hw_common_reg.h"
#include "rom.h"
#include "utils.h"
#include "uart.h"

#define SYS_CLK_FREQ 80000000

// Pointers to the two user-provided windows
static q15_t *g_pPing = 0;
static q15_t *g_pPong = 0;

static volatile uint32_t g_uiCount = 0;
static volatile uint32_t g_uiSize = 0;
static volatile uint8_t  g_activeBuffer = 0; // 0 for Ping, 1 for Pong

// Flags to communicate with the main loop
static volatile uint8_t  g_bPingReady = 0; // First one can be read from
static volatile uint8_t  g_bPongReady = 0; // Second one can be read from
static volatile uint8_t  g_bOverrunOccurred = 0; // Sample overrun occured
static volatile uint8_t  g_bSoftwareOverrun = 0; // Buffer 1 not processed before Buffer 2 full

// Internal Interrupt Service Routine
void ADCIntHandler() {
    uint32_t ulStatus;

    ulStatus = MAP_ADCIntStatus(ADC_BASE, ADC_CH_3);
    MAP_ADCIntClear(ADC_BASE, ADC_CH_3, ulStatus);
    // Check for hardware FIFO overflow

    if ((g_activeBuffer == 0 && g_bPingReady) || (g_activeBuffer == 1 && g_bPongReady)) {
        g_bSoftwareOverrun = 1; // New flag: CPU is too slow!

        // This clears the hardware flag so we don't get trapped in an infinite loop!
        MAP_ADCFIFORead(ADC_BASE, ADC_CH_3);

        return; // Drop this sample to avoid corrupting the buffer being read by FFT
    }

    // Get the pointer to the currently filling buffer
    q15_t *pCurrent = (g_activeBuffer == 0) ? g_pPing : g_pPong;

    if (pCurrent != 0 && g_uiCount < g_uiSize) {
        // Read sample: Shifted right by 2 per CC3200 specs (12-bit value)
        pCurrent[g_uiCount] = (q15_t)((MAP_ADCFIFORead(ADC_BASE, ADC_CH_3) >> 2) & 0x0FFF);
        g_uiCount++;

        // Window is full so swap the buffers immediately
        if (g_uiCount >= g_uiSize) {
            if (g_activeBuffer == 0) {
                g_bPingReady = 1;
                g_activeBuffer = 1; // Switch to Pong
            } else {
                g_bPongReady = 1;
                g_activeBuffer = 0; // Switch to Ping
            }
            g_uiCount = 0; // Reset index for the new buffer

            // If we just filled Ping, but Pong is STILL marked as ready,
            // it means the main loop is too slow and hasn't processed Pong yet.
            if ((g_activeBuffer == 1 && g_bPongReady) || (g_activeBuffer == 0 && g_bPingReady)) {
                g_bOverrunOccurred = 1;
            }
        }
    }
}


// RUNS THE CONFIGURATION
void SetupADCMic(uint32_t sampleRate) {
    uint32_t uiTimerTicks;
    uint32_t uiActualSampleRate;

    //  Enable Clocks and Peripheral
    MAP_PRCMPeripheralClkEnable(PRCM_ADC, PRCM_RUN_MODE_CLK);
    MAP_ADCEnable(ADC_BASE);
    MAP_ADCChannelEnable(ADC_BASE, ADC_CH_3);

    uiTimerTicks = SYS_CLK_FREQ / sampleRate;

    // Calculate actual resulting frequency (to see rounding error)
    uiActualSampleRate = SYS_CLK_FREQ / uiTimerTicks;

    //  Diagnostics Report
    Report("\n\r--- ADC INITIALIZATION DIAGNOSTICS ---\n\r");
    Report("System Clock: %d Hz\n\r", SYS_CLK_FREQ);
    Report("Target Sample Rate: %d Hz\n\r", sampleRate);
    Report("Actual Sample Rate: %d Hz\n\r", uiActualSampleRate);
    Report("Timer Reload Ticks: %d\n\r", uiTimerTicks);

    // Safety Check: Verify Nyquist/Human Hearing range for FFT
    if (uiActualSampleRate < 20000) {
        Report("WARNING: Sample rate low for high-quality audio FFT.\n\r");
    }

    Report("Status: Registering Interrupt and Starting Hardware Timer...\n\r");


    // 4. Final Configuration
    MAP_ADCTimerConfig(ADC_BASE, uiTimerTicks);
    Report("Status: Fixing Base...\n\r");
    MAP_ADCIntRegister(ADC_BASE, ADC_CH_3, ADCIntHandler);
    Report("Status: Setting priority...\n\r");
    MAP_IntPrioritySet(INT_ADCCH1, 0x20);
    Report("--------------------------------------\n\r\n\r");
}




// FULL VARIABLE RESET N STARTUP
void StartADCSampling(q15_t *ping_buffer, q15_t *pong_buffer, uint32_t window_size) {
    g_pPing = ping_buffer;
    g_pPong = pong_buffer;
    g_uiSize = window_size;
    g_uiCount = 0;
    g_activeBuffer = 0;
    g_bPingReady = 0;
    g_bPongReady = 0;
    g_bOverrunOccurred = 0;

    MAP_ADCIntEnable(ADC_BASE, ADC_CH_3, ADC_FIFO_FULL | ADC_FIFO_OVERFLOW);
    MAP_ADCTimerEnable(ADC_BASE);
}

int CheckBufferReady() {
    if (g_bOverrunOccurred) return -1;
    if (g_bPingReady) return BUFFER_PING;
    if (g_bPongReady) return BUFFER_PONG;
    return BUFFER_NONE;
}

void ClearBufferFlag(int buffer_id) {
    if (buffer_id == BUFFER_PING) g_bPingReady = 0;
    if (buffer_id == BUFFER_PONG) g_bPongReady = 0;
}

void ClearOverrunFlag() {
    g_bOverrunOccurred = 0;
}
