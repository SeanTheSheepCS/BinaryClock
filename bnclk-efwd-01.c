/**********************************************************************
* Supporting functions for Binary Clock
**********************************************************************/

/************************ Revision History ****************************
YYYY-MM-DD  Comments
-------------------------------------------------------------------------------------------
2015-02-DD  First release. 

************************************************************************/

#include "io430.h"
#include "typedef_MSP430.h"
#include "intrinsics.h"
#include "bnclk-efwd-01.h"
#include "main.h"

/******************** External Globals ************************/
/* Globally available variables from other files as indicated */


/******************** Program Globals ************************/
/* Global variable definitions intended for scope across multiple files */
fnCode_type GG_fpCLOCKSM;      //the state machine function pointer
int GG_u8Second_Counter = 0;                       //the second counter

/******************** Local Globals ************************/
/* Global variable definitions intended only for the scope of this file */
u8 LG_u8Minute_Counter = 0;                       //the minute counter
u8 LG_u8Hour_Counter = 12;                        //the hour counter
u8 LG_u8PM = 1;                                   //AM/PM counter, when LSB is 1 output is PM, 0>AM
u8 LG_u8Flash = 1;                                //on/off for when flashing outputs


/******************** Function Definitions ************************/
/*------------------------------------------------------------------------------
Function: ClockSM_Start

Description:This function is used for start-up to flash 12:00 PM on the display until
a button is pressed to set the correct time.  The program should never return to this function
unless the device is reset
 
Requires:Timer A has been started

Promises:To flash 12:00PM until a button is pressed

*/
void ClockSM_Start()
{
  if (LG_u8Flash == 3)
  {              //display is off turn it back on
    LG_u8Flash = 0;
    LG_u8Hour_Counter = 12;       //just in case this gets mixed up somehow
    LG_u8Minute_Counter = 0;      //ditto
    Update_Display();             //Turn on the LED's
  }
  else
  {                           //display is on turn if back off
    LG_u8Flash++;
    P1OUT &= Port1_Clear_Mask;  /*these statments turn off all LED's except for TICK*/
    P2OUT &= Port2_Clear_Mask;
    P3OUT &= Port3_Clear_Mask;    
  }
  
  Poll_Buttons();                 /*this is the only way to break from the start routine*/
  __bis_SR_register(LPM3_bits);   //sleep until timer A expires

} /* end ClockSM_Start */

/*------------------------------------------------------------------------------
Function: ClockSM_Tick

Description: This function toggles the Tick LED every 500ms, It also checks if
Second_Counter has overflown prompting a display update as well as polling the buttons once every 500ms
 
Requires: Timer A has been started

Promises: To flash the TICK LED ~ every 1s, and keep the current time.
If the buttons are held for an extended time ClockSM_Tick should force the time to recover in a few cycles.
*/
void ClockSM_Tick()
{
  /*Check if the time needs to be updated*/
  if(GG_u8Second_Counter>=240)
  {   //currently using 500ms update cycles
    GG_u8Second_Counter -= 240;   //this should set us to zero but catches any missed half second cycles
    LG_u8Minute_Counter++;
    Time_Rollover();
    Update_Display();
  }
  
  /*Toggle the TICK LED*/
  if (LG_u8Flash==3){
    LG_u8Flash = 0;
    P3OUT |= P3_4_PIMO_TICK;           //Turn on TICK
  }
  else
  {
    LG_u8Flash++;
    P3OUT &= ~P3_4_PIMO_TICK;          //Turn off TICK
  }
  
  Poll_Buttons();
  __bis_SR_register(LPM3_bits);   //sleep until timer A expires
  
} /* end ClockSM_Tick */


/*------------------------------------------------------------------------------
Function: Update_Display

Description: drives the respective LEDs for the current time
 
Requires: Minute_Counter, Hour_Counter, and LG_u8PM have correct values

Promises: To drive Hour, Minute and PM LEDs

*/
void Update_Display()
{
  u8 Port_Update_Value = 0;

  /*Port 1 LED driver  output port is x x x x m0 m1 m2 m3*/
  for(u8 i = 0; i < 4; i++)
  {
    Port_Update_Value |= (((LG_u8Minute_Counter<<i) & Port1_Update_Mask)>>(3-i));
  }
  
  //port update value should now be 0b0000 m0 m1 m2 m3
  P1OUT &= Port1_Clear_Mask;
  P1OUT |= Port_Update_Value;
  
  /*Port 2 LED driver  output port is x x x m4 m5 h3 x x  done directly as a loop isn't worth it here*/
  P2OUT &= Port2_Clear_Mask;                            // clears m4, m5 and h3
  P2OUT |= (LG_u8Minute_Counter >> 2) & P2_3_MINUTE_5;    // shift m5 from bit 5 to bit 3, mask, drive
  P2OUT |= LG_u8Minute_Counter & P2_4_MINUTE_4;         // whoo! m4 is already in the right spot, mask, drive
  P2OUT |= (LG_u8Hour_Counter >> 1) & P2_2_HOUR_3;        //  shift h3 from bit 3 to bit 2, mask, drive
  
  /*Port 3 LED driver output port is x PM x x x h0 h1 h2 */
  Port_Update_Value = 0;  // zero our update value
  for(u8 i = 0; i < 3; i++)
  {
    Port_Update_Value |= (((LG_u8Hour_Counter<<(i)) & Port3_Update_Mask)>>(2-i));
  }
  
  // we do PM manually It increments every 12 hours so mask for the LSB to get AM/PM
  P3OUT &= Port3_Clear_Mask;
  Port_Update_Value |= ((LG_u8PM<<5)&P3_5_POMI_PM_IND);  
  
  //port update value should now be 0 PM 000 h0 h1 h2
  P3OUT |= Port_Update_Value;
  
} /* end Update_Display */


/*------------------------------------------------------------------------------
Function: ClockSM_Button_Press

Description: Debounces the buttons (on a ~250ms debounce which may need to be changed)
Increments the minute and hour counters for button 1 and 2 respectively
 
Requires: 
  - PIMO and POMI are not being used to communication currently
  - P3_5, 3_4, 2_1 are configured as inputs

Promises: 
  - Increases the minute or hour 2X per second as requested

*/
void ClockSM_Button_Press()
{
  if(!(P2IN&P2_1_BUTTON_0))
  {
    //currently does nothing other than start ticking the clock if we were in START
  }
  else if(!(P3IN&P3_7_BUTTON_1))
  {
    LG_u8Minute_Counter++;  //button one increases the minute
    GG_u8Second_Counter = 0; // and clears the current second so timing the button press give 500ms accuracy approximately
  }
  else if(!(P3IN&P3_6_BUTTON_2))
  {
    LG_u8Hour_Counter++;  //button two increases the hour
  }
  
  Time_Rollover();
  Update_Display();
  GG_fpCLOCKSM = ClockSM_Tick;
  
} /* end ClockSM_Button_Press */


/*------------------------------------------------------------------------------
Function: ClockSM_LP_Sleep

Description: Effectively the same as Tick but doesn't poll the buttons or update the display
until power returns

Requires: 
  - LP_IND is low
  - Timer A has been started

Promises: 
  - Keeps the current time when power is lost and use the least amount of power possible

*/
void ClockSM_LP_Sleep()
{
  //check if the power is back
  if(P2IN&P2_5_LOST_POWER_IND)
  {
    GG_fpCLOCKSM = ClockSM_Tick;
    Update_Display();
  }
  
    /*Check if the time needs to be updated*/
  if(GG_u8Second_Counter >= 240)
  {
    GG_u8Second_Counter -= 240;
    LG_u8Minute_Counter++;
    Time_Rollover();
  }
  
  __bis_SR_register(LPM3_bits); //sleep until timer A expires
  
} /* end ClockSM_LP_Sleep */


/*------------------------------------------------------------------------------
Function: Clock_Initialize

Description: Starts Timer A on a 500ms Loop which generates and interrupt each time
Sets the ports as inputs/outputs as needed
enables the interrupts needed
 
Requires: 

Promises: 
*/
void Clock_Initialize()
{
  /*  This is mostly redundant as the system is set up in cstartup.s43
      Only setting TACCR0, the Sel2, and starting the timer are essential
  Configure Ports as Digital I/O*/
  P1SEL = Port1_Sel;
  P2SEL = Port2_Sel;
  P3SEL = Port3_Sel;
  P1SEL2 = Port1_Sel2;
  P2SEL2 = Port2_Sel2;
  
  /*Initialize port directionality*/
  P1DIR = Port1_Direction;
  P2DIR = Port2_Direction;
  P3DIR = Port3_Direction;
  
  __bis_SR_register(GIE);
  
  /*Set the 500 ms Timer limit and start the timer*/
  TACCR0 = TIME_250MS;
  TACTL = TIMERA_INITIALIZE;
 
} /* end Clock_Initialize */

/*-----------------------
-------------------------------------------------------
Function: Poll_Buttons

Description:  Checks if button 0-2 have been pressed
 
Requires: 

Promises:

*/
void Poll_Buttons()
{
  if(!(P3IN&P3_7_BUTTON_1) || !(P2IN&P2_1_BUTTON_0) || !(P3IN & P3_6_BUTTON_2))
  {
    GG_fpCLOCKSM = ClockSM_Button_Press;
  }
  
} /* end Poll_Buttons() */


/*------------------------------------------------------------------------------
Function: Time_Rollover

Description:  Corrects for time rollover
 
Requires: 

Promises: To correct the current time into 12 hour format

*/
void Time_Rollover()
{
  if(LG_u8Minute_Counter >= 60)
  {
    LG_u8Minute_Counter -= 60;
    LG_u8Hour_Counter++;
  }
  
  if(LG_u8Hour_Counter >= 13)
  {
    LG_u8Hour_Counter -= 12;
    LG_u8PM ++;
  }
  
  if(LG_u8PM >= 2)
  {
    LG_u8PM = 0;
  }//end pm rollover
  
} /* end Time_Rollover() */

