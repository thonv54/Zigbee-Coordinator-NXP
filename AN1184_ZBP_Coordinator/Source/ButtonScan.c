/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_endpoint.c
 *
 * DESCRIPTION:			End Point Event Handler
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
#include "jendefs.h"
#include <dbg.h>
#include <stdio.h>
#include <zps_apl_af.h>
#include <os_gen.h>
#include <app_timer_driver.h>
#include "AppHardwareApi.h"

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


#define 	ButtonOn	1
#define 	ButtonOff	0
#define	 	AllOn		0b111
#define 	AllOff		0b000
#define 	InputButtonOn		0
#define 	OutputLedOn			1
#define 	LedOn			1

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
/****************************************************************************/

/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
 union CBF{
	struct {
		u_char button_0 :1;
		u_char button_1 :1;
		u_char button_2 :1;

	} b;
	u_char all :3;
} CurrentButtonStatus;


 union LBF{
	struct {
		u_char button_0 :1;
		u_char button_1 :1;
		u_char button_2 :1;
	} b;
	u_char all :3;
} LastButtonStatus;


typedef enum{
	InitState,
	RunningState,
}ButtonScanTaskState_enum;

uint8 LedState = 0;
uint8 ButtonScanTaskState = InitState;
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/



/****************************************************************************
 *
 * NAME: Button
 *
 * DESCRIPTION:
 * End Point event handling
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void SetIoInputDirection(uint32 PinMark){
	vAHI_DioSetDirection(PinMark ,0);
}
void SetIoOutputDirection(uint32 PinMark){
	vAHI_DioSetDirection(0, PinMark);
}


void SetledOn(void){
#if OutputLedOn == 1
		vAHI_DioSetOutput(E_AHI_DIO13_INT,0);
#else
		vAHI_DioSetOutput(0,E_AHI_DIO13_INT);
#endif
}


void SetLedOff(void){
#if OutputLedOn == 1
		vAHI_DioSetOutput(0,E_AHI_DIO13_INT);

#else
		vAHI_DioSetOutput(E_AHI_DIO13_INT,0);
#endif
}
void InitButton(void){
	CurrentButtonStatus.all = AllOff;
	LastButtonStatus.all = AllOff;
	SetIoInputDirection(E_AHI_DIO8_INT);
	SetIoOutputDirection(E_AHI_DIO13_INT);
	SetLedOff();
}
void LedRefresh(void){
	if(LedState == LedOn){
		SetledOn();
	}
	else{
		SetLedOff();
	}
}
void RunningButtonScan(void){
	if(((u32AHI_DioReadInput() & E_AHI_DIO8_INT) >>  8) == InputButtonOn){

		CurrentButtonStatus.b.button_0 = ButtonOn;
	}
	else{
		CurrentButtonStatus.b.button_0 = ButtonOff;
	}

	if ((CurrentButtonStatus.b.button_0 == ButtonOn) && (LastButtonStatus.b.button_0 == ButtonOff)){
		DBG_vPrintf(TRACE_APP, "button 0 press \n\r");
		LedState^=1;
		LedRefresh();
	}
	else if((CurrentButtonStatus.b.button_0 == ButtonOn) && (LastButtonStatus.b.button_0 == ButtonOn)){

	}
	else if ((CurrentButtonStatus.b.button_0 == ButtonOff) && (LastButtonStatus.b.button_0 == ButtonOn)){
		DBG_vPrintf(TRACE_APP, "button 0 unpress \n\r");

	}
}



OS_TASK(ButtonScan)
{


	switch (ButtonScanTaskState) {
		case InitState:
			{
				InitButton();
				ButtonScanTaskState = RunningState;
			}
			break;
		case RunningState:
			{
				RunningButtonScan();
			}
			break;
		default:
			break;
	}

	LastButtonStatus.all = CurrentButtonStatus.all;
	if (OS_eGetSWTimerStatus(ButtonScanTimer) == OS_E_SWTIMER_EXPIRED)
	{
		OS_eContinueSWTimer (ButtonScanTimer, APP_TIME_MS(100), NULL);
	}
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
