/**********************************************************************
File name:	main.c

Main program file for Binary Clock 
A simple program that displays the current time in binary format:
HHHH MMMMMM (PM)
The current time can be set by the use of the buttons.
In the event of power loss the device will stop powering the display but the backup
battery will keep the current time until power is restored

Future additions will likely include an alarm with the ability to set, snooze and turn off
said alarm
**********************************************************************/




/************************ Revision History ****************************
YYYY-MM-DD  Checksum  Comments
-------------------------------------------------------------------------------------------
2015-02-dd            First release. 

************************************************************************/

#include "io430.h"
#include "typedef_MSP430.h"
#include "intrinsics.h"
#include "main.h"
#include "bnclk-efwd-01.h"


/************************ External Program Globals ****************************/
/* Globally available variables from other files as indicated */
extern fnCode_type GG_fpCLOCKSM;                 /* From bnclk-efwd-01.c */

extern int GG_u8Second_Counter;            /* From bnclk-efwd-01.c */


/************************ Program Globals ****************************/
/* Global variable definitions intended for scope of multiple files */


/************************ Main Program ****************************/
/* From cstartup.s43, the processor is running from the ACLK, TimerA and I/O lines are configured by Clock_Initialize. */

int main(void)
{

  /* Enter the state machine where the program will remain unless power cycled */

  Clock_Initialize();               //initialize the ports, enable interupts and start the clock
  GG_fpCLOCKSM = ClockSM_Start;

  while(1)
  {
    //the state machine starts in the start function then upon button press
    //enters the tick function and stays there unless power is lost
	  GG_fpCLOCKSM();
  } 
} /* end main */


/************************ Interrupt Service Routines ****************************/
#pragma vector = PORT2_VECTOR
__interrupt void Port2ISR(void)
/* Handles interupt caused by loss of power returning to LP_Sleep state with all outputs off */
{
  GG_fpCLOCKSM = ClockSM_LP_Sleep;
  P2IFG=0x00;
//  P1OUT=Port1_LP_Sleep;
//  P2OUT=Port2_LP_Sleep;
//  P3OUT=Port3_LP_Sleep;
  //asm("BIC #0x0010,4(SP)"); dont wake up the processor continue to sleep until the 500ms timer expires
} /* end Port1ISR */


/*----------------------------------------------------------------------------*/
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerAISR(void)
{
  asm("BIC #0x00D0,0(SP)"); //this is a bit clear at 0(SP) of what appears to be all the bits? 
  GG_u8Second_Counter++;
  TACTL = TIMERA_INT_CLEAR_FLAG;
  // can I just write 'LPM0_EXIT;' as defined in io430x21x2?
//  LPM3_EXIT;
  
} // end half second tick ISR

