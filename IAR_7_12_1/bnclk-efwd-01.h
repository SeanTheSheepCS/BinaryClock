/**********************************************************************
* Header file for Binary Clock
**********************************************************************/

/************************ Revision History ****************************
YYYY-MM-DD  Comments
-------------------------------------------------------------------------------------------
2015-02-DD  Modified from Blinky

************************************************************************/

#ifndef __BNCLK_HEADER
#define __BNCLK_HEADER

/****************************************************************************************
Constants
****************************************************************************************/

#define true 1
#define false 0

#define LEDS_FOR_HOURS (u8)4
#define LEDS_FOR_MINUTES (u8)6
#define CUSTOM_CODE_ENABLED 1

/* Timing constants */
#define TIME_250MS          (u16)8191  /* Taccro for X = (0.25s * (32768Hz)) - 1; max = 65535
This depends on a 32768Hz oscillator and usage of the divider*/


/****************************************************************************************
Hardware Definitions
****************************************************************************************/
/* Port 1 pins */
#define P1_0_MINUTE_3            0x01
#define P1_1_MINUTE_2            0x02
#define P1_2_MINUTE_1            0x04
#define P1_3_MINUTE_0            0x08
#define P1_4_TCK_UNUSED          0x10
#define P1_5_TMS_UNUSED          0x20
#define P1_6_TDI_UNUSED          0x40
#define P1_7_TDO_UNUSED          0x80

/* Port 2 pins */
#define P2_0_NC_UNUSED           0x01
#define P2_1_BUTTON_0            0x02
#define P2_2_HOUR_3              0x04
#define P2_3_MINUTE_5            0x08
#define P2_4_MINUTE_4            0x10
#define P2_5_LOST_POWER_IND      0x20
#define P2_6_XIN                 0x40
#define P2_7_XOUT                0x80


/* Port 3 pins */
#define P3_0_HOUR_2              0x01
#define P3_1_HOUR_1              0x02
#define P3_2_HOUR_0              0x04
#define P3_3_BUZZER              0x08
#define P3_4_PIMO_TICK           0x10
#define P3_5_POMI_PM_IND         0x20
#define P3_6_BUTTON_2            0x40
#define P3_7_BUTTON_1            0x80

/* LED driver masks */
#define Port1_Update_Mask        0x08  //use a mask in bit 3 to simplify the shift opperations in update display
#define Port1_Clear_Mask         0xF0  //~(& of port 1 minute pins)
#define Port2_Clear_Mask         0xE3  //~(& of m5, m4 and h3)
#define Port3_Update_Mask        0x04  //use a mask in bit 2 to simplify the shift operations
#define Port3_Clear_Mask         0xD8  //~(& of h2, h1, h0, PM)

/*Port Directionality  0 input 1 output*/
#define Port1_Direction  0x0F    //0000 1111
#define Port2_Direction  0x1C    //0001 1100
#define Port3_Direction  0x3F    //0011 1111

/*Port Digital IO Selection*/
#define Port1_Sel    0xF0    //1111 0000
#define Port2_Sel    0xC0    //1100 0000 6 and 7 are set to enable XIN and XOUT
#define Port3_Sel    0x00    //0000 0000
#define Port1_Sel2   0x00    //0000 0000 not sure if I want this or 1111 0000 for use of JTAG I can't find it in the users guide
#define Port2_Sel2   0xC0    //1100 0000 6 and 7 are set to enable XIN and XOUT

/*Type defines*/
typedef enum
{
  FLASH_OFF = 0,
  FLASH_ON = 1,
}Flash_Status;

#define Seconds_Per_Minute 60



/* Setup constants */
#define TIMERA_INITIALIZE  0x0116
/* Value for TACTL to set up Timer A to be running with the interrupt enabled
    <15-10> [000000] not used
    <9-8> [01] ACLK Timer A clock source
    <7-6> [00] Input divider /1
    <5-4> [01] Up mode
    <3> [0] not used
    <2> [1] Reset the timer module
    <1> [1] Enable the timer interrupt
    <0> [0] Clear the interrupt flag
*/

#define TIMERA_INT_CLEAR_FLAG  0x0112	
/* Value for TACTL to Clear the Timer A Flag:
    <15-10> [000000] not used
    <9-8> [01] ACLK Timer A clock source
    <7-6> [00] Input divider /1
    <5-4> [01] Up mode
    <3> [0] not used
    <2> [0] !Don't Reset the timer module!
    <1> [1] Keep the interrupt enabled
    <0> [0] Clear the interrupt flag
*/


/************************ Function Declarations ****************************/
void Clock_Initialize();  /*Starts the timer 500ms loop to run forever*/
void Poll_Buttons();       /*Checks if buttons are pressed and makes ClockSM_Button_Press the next state if they are*/
void Time_Rollover();     /*adjusts the minute, hour and PM to stay in standard format e.g. 13:62PM -> 2:02AM*/
void Update_Display();      /*Change the display LEDs (hours, Minutes and PM */
void Update_Display_Hours(); /*Change the display LEDS but just for hours */

/****************************************************************************************
State Machine Functions
****************************************************************************************/
void ClockSM_Start();               /*Flash LED's displaying 12:00PM until button press occurs */
void ClockSM_Tick();                /*Check the second counter, flash the Tick LED, Poll the buttons, sleep then branch accordingly */
void ClockSM_Button_Press();        /*hour ++, Minute ++ or do nothing for Buttons 2-0 respectivly */
void ClockSM_LP_Sleep();            /*similar to Tick but only update the display once power is returned, ignor buttons*/

#endif /* __BNCLK_HEADER */
