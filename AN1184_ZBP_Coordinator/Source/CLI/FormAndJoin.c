/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:          	FormAndJoin.c
 *
 * DESCRIPTION:        	Exception routines
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
#include <os.h>
#include <dbg.h>
#include "os_gen.h"
#include "UART.h"
#include "AppHardwareApi.h"
#include "string.h"
#include "app_timer_driver.h"
#include "PDM.h"
#include "app_common.h"
#include "app_coordinator.h"
#include "PDM_IDs.h"

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
#define CLI_NetworkFindUnused	CLI_NetworkForm

typedef enum{
	NetworkForm,
	NetworkLeave,
	NetworkFindUnused,
}CLI_CommandEnum;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


PUBLIC void CLI_NetworkForm(void){
	ZPS_eAplAfInit();
    ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
    if (ZPS_E_SUCCESS == eStatus)
    {
        s_eDeviceState = E_NETWORK_FORMATION;
    }
    else
    {
        DBG_vPrintf(TRACE_APP, "APP: ZPS_eZdoStartStack() failed error %d", eStatus);
    }
}

PUBLIC void CLI_NetworkLeave(void){
    PDM_vDeleteAllDataRecords();
    DBG_vPrintf(TRACE_APP, "APP: Deleting all records from flash\n");
    s_eDeviceState = E_NETWORK_FORMATION;
    PDM_eSaveRecordData(PDM_ID_APP_COORD,
                    	E_NETWORK_INIT,
                    	sizeof(E_NETWORK_INIT));
}

PUBLIC void CLI_Pjoin(uint8 time){
	ZPS_teStatus eStatus = ZPS_eAplZdoPermitJoining(time);
	if(ZPS_E_SUCCESS == eStatus){
		DBG_vPrintf(TRACE_APP, "Permit Join 0x%x \n",time);
	}
	else{
		DBG_vPrintf(TRACE_APP, "Permit Join Error 0x%x :  \n",eStatus);
	}
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
