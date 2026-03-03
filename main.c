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

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
static void LEDBinaryCounter();
static void BoardInit(void);


static void
DisplayBanner()
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t  CC3200 GPIO Application       \n\r");
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
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


    while(1)
    {
        // DO NOTHING
    }
    return 0;
}

