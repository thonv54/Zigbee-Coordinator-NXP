/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:          	app_exceptions.c
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
#include "FormAndJoin.h"
#include "stdlib.h"

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
#define UartCommandBuffer		2
#define UartPacketLengthMax		20

//typedef enum{
//	NetworkForm,
//	NetworkLeave,
//	NetworkFindUnused,
//}CLI_CommandEnum;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
//PRIVATE CLI_CommandEnum CLI_Command;

uint8* UartPacketData[UartCommandBuffer][UartPacketLengthMax];
uint8 PacketCnt	= 0;
uint8 UartCommandNumber;
uint8 NextUartCommandNumber;
uint8 UartCommandPos;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void removeSubstring(char *s,const char *toremove)
{
  while( s=strstr(s,toremove) )
    memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}

void InitUart(void){
	uint8 i;
	vAHI_UartSetInterrupt(E_AHI_UART_0,FALSE,FALSE,FALSE,TRUE,E_AHI_UART_FIFO_LEVEL_1);
	for(i=0;i<UartCommandBuffer;i++){
		memset(UartPacketData[i], '\0',UartPacketLengthMax);
	}
}
void UartGetPacketData(void){

	char data;
	char tempdata[2] = "\0";
	data = u8AHI_UartReadData(E_AHI_UART_0);
	tempdata[0] = data;

	strcat(UartPacketData[UartCommandPos],tempdata);
	if(strcmp(strrchr(UartPacketData[UartCommandPos],'\n'), '\0' )){

		UartCommandPos++;												// vi tri luu ban tin toi
		if (UartCommandPos >= UartCommandBuffer) {
			UartCommandPos = 0;
		}
		UartCommandNumber++;
		if (UartCommandNumber >= UartCommandBuffer) {					// so ban tin chua dc xu ly
			UartCommandNumber = 0;
		}
	}


}

void CompleteCommand(void) {
	memset(UartPacketData[NextUartCommandNumber], '\0',UartPacketLengthMax);
	UartCommandNumber--;							// so ban tin con lai chua dc xu ly
	if (UartCommandPos >= UartCommandNumber) {
		NextUartCommandNumber = UartCommandPos - UartCommandNumber; // neu so ban tin chua duoc xu li nho hon so thu tu ban tin
	} else {
		NextUartCommandNumber = UartCommandBuffer + UartCommandPos - UartCommandNumber;	// co ban tin chua dc xu ly lon so thu tu ban tin--> quay tro lai cuoi
	}
}
OS_ISR(Uart0GetData)
{
	UartGetPacketData();
}
OS_TASK(UartCommandExec){
	uint16 u16DataBytesRead;

	if (UartCommandNumber != 0){

		// xu li lenh dang co


		if(strncmp(UartPacketData[NextUartCommandNumber], "network form", strlen("network form")) == 0){
			DBG_vPrintf(TRACE_APP, "network form.... \n");
			CLI_NetworkForm();
		}

		else if(strncmp(UartPacketData[NextUartCommandNumber], "network find unused", strlen("network find unused")) == 0){

			DBG_vPrintf(TRACE_APP, "network find unused.... \n");
			CLI_NetworkForm();
		}

		else if(strncmp(UartPacketData[NextUartCommandNumber], "network leave", strlen("network leave")) == 0){
			DBG_vPrintf(TRACE_APP, "network leave... \n");
			CLI_NetworkLeave();

		}
		else if(strncmp(UartPacketData[NextUartCommandNumber], "network pjoin", strlen("network pjoin")) == 0){
			{
				char dataStr[5] = "\0";
				uint8 ptime;
				removeSubstring(UartPacketData[NextUartCommandNumber],"network pjoin ");
				memcpy(dataStr,UartPacketData[NextUartCommandNumber],4);
				if(strncmp(dataStr, "0x", strlen("0x")) == 0){
					ptime = (uint8)strtol(dataStr, '\0', 0);
					CLI_Pjoin(ptime);
				}
				else{
					DBG_vPrintf(TRACE_APP,"Input Format Error \n");
				}
//				DBG_vPrintf(TRACE_APP,"data is %d \n",ptime);
//				DBG_vPrintf(TRACE_APP,"data is %s \n",UartPacketData[NextUartCommandNumber]);

			}
		}

		CompleteCommand();
	}



	if (OS_eGetSWTimerStatus(UartCommandExecTimer) == OS_E_SWTIMER_EXPIRED)
	{
		OS_eContinueSWTimer (UartCommandExecTimer, APP_TIME_MS(100), NULL);
	}
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
