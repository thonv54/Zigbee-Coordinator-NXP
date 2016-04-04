/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_start.c
 *
 * DESCRIPTION:			Coordinator Initialization and Startup
 *
 *****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5169, JN5168,
 * JN5164, JN5161].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2015. All rights reserved
 *
 ****************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppApi.h>
#include <pwrm.h>
#include <os.h>
#include <dbg.h>
#include <dbg_uart.h>
#include <app_exceptions.h>
#include <app_pdm.h>
#include <app_coordinator.h>
#include <zps_nwk_pub.h>

#include "os_gen.h"
#include "pdum_gen.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_APP
	#define TRACE_APP 	FALSE
#else
	#define TRACE_APP 	TRUE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vInitialiseApp(void);
PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern void *_stack_low_water_mark;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/*****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Entry point for application from a cold start.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PUBLIC void vAppMain(void)
{
	/* Wait until FALSE i.e. on XTAL  - otherwise uart data will be at wrong speed */
    while (bAHI_GetClkSource() == TRUE);
    /* Now we are running on the XTAL, optimise the flash memory wait states. */
    vAHI_OptimiseWaitStates();

    /* Initialise the debug diagnostics module to use UART0 at 115K Baud;
     * Do not use UART 1 if LEDs are used, as it shares DIO with the LEDS
     */
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "\n\nAPP: Power Up\n");

    /*
     * Initialise the stack overflow exception to trigger if the end of the
     * stack is reached. See the linker command file to adjust the allocated
     * stack size.
     */
    vAHI_SetStackOverflow(TRUE, (uint32)&_stack_low_water_mark);

    /*
     * Catch resets due to watchdog timer expiry. Comment out to harden code.
     */
    if (bAHI_WatchdogResetEvent())
    {
        DBG_vPrintf(TRACE_APP, "APP: Watchdog timer has reset device!\n");
        vAHI_WatchdogStop();
        while (1);
    }

    /* Initialize ROM based software modules */
    #ifndef JENNIC_MAC_MiniMacShim
        (void)u32AppApiInit(NULL, NULL, NULL, NULL, NULL, NULL);
    #endif
    /* Define HIGH_POWER_ENABLE to enable high power module */
    #ifdef HIGH_POWER_ENABLE
        vAHI_HighPowerModuleEnable(TRUE, TRUE);
    #endif

    /* start the RTOS */
    OS_vStart(vInitialiseApp, vUnclaimedInterrupt, vOSError);

    /* idle task commences here */
    while (TRUE)
    {
        /* Re-load the watch-dog timer. Execution must return through the idle
         * task before the CPU is suspended by the power manager. This ensures
         * that at least one task / ISR has executed with in the watchdog period
         * otherwise the system will be reset.
         */
        vAHI_WatchdogRestart();

        /*
         * suspends CPU operation when the system is idle or puts the device to
         * sleep if there are no activities in progress
         */
        PWRM_vManagePower();
    }
}

/*****************************************************************************
 *
 * NAME: vAppRegisterPWRMCallbacks
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
void vAppRegisterPWRMCallbacks(void)
{
    /* nothing to register as device does not sleep */
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/*****************************************************************************
 *
 * NAME: vInitialiseApp
 *
 * DESCRIPTION:
 * Initialises JenOS modules and application.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitialiseApp(void)
{
    /* Initialise JenOS modules. Initialise Power Manager even on non-sleeping nodes
     * as it allows the device to doze when in the idle task
     */
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);

    /*
     *  Initialise the PDM, use an application supplied key (g_sKey),
     *  The key value can be set to the desired value here, or the key in eFuse can be used.
     *  To use the key stored in eFuse set the pointer to the key to Null, and remove the
     *  key structure here.
     */
	#ifdef PDM_EEPROM
    	PDM_eInitialise(0, NULL);
    	PDM_vRegisterSystemCallback(vPdmEventHandlerCallback);
	#else
    	PDM_vInit(7, 1, 64 * 1024 , NULL, NULL, NULL, &g_sKey);
	#endif

    /* Initialise Protocol Data Unit Manager */
    PDUM_vInit();

    /* Register callback that provides information about stack errors */
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    /* initialise application */
    APP_vInitialiseCoordinator();
}

/****************************************************************************
 *
 * NAME: vfExtendedStatusCallBack
 *
 * DESCRIPTION:
 * ZPS extended error callback .
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
	DBG_vPrintf(TRACE_APP, "ERROR: Extended status %x\n", eExtendedStatus);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/