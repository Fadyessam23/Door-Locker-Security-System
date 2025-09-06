/*
 * Module: Timer1
 *
 * File Name: Timer1.c
 *
 * Description: Source file for Timer 1 AVR driver
 *
 * Author: Fady Essam
 *
 */

#include "Timer1.h"
#include "common_macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>


/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

/* Global variables to hold the address of the call back function in the application */

void (*call_back)(void);

// Timer1 configuration function
Timer1_ConfigType * Timer1_Config_Ptr;


/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/

// interrupt service routine for timer1 in overflow (normal) mode
ISR(TIMER1_COMPA_vect)
{
	call_back();
	TCNT1=Timer1_Config_Ptr->initial_value;

}

// interrupt service routine for timer1 in compare (CTC) mode
ISR(TIMER1_OVF_vect)
{
	call_back();
	TCNT1=Timer1_Config_Ptr->initial_value;
}


/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description:
 * Function to initialize the Timer driver
 */

void TIMER1_init(const Timer1_ConfigType * Config_Ptr)
{
	// To Enable Interrupts
	SREG|=(1<<7);
	Timer1_Config_Ptr = Config_Ptr;
	/*
	 * For TCCR1B register :
	 * WGM12 = 1 (Compare mode) or 0 (Normal mode)
	 * CS12 : CS10 = prescaler value
	 */
	TCCR1B|=(Config_Ptr->prescaler>>0);
	TCNT1=Config_Ptr->initial_value;
	//if Timer1 is in normal (overflow) mode :
	if(Config_Ptr->mode==NORMAL)
	{
		TCCR1B&=~(1<<WGM12);
		//Overflow Interrupt Enable
		TIMSK|=(1<<TOIE1);

	}
	//if Timer1 is in compare (CTC) mode :
	else if(Config_Ptr->mode==COMPARE)
	{
		TCCR1B|=(1<<WGM12);
		OCR1A = Config_Ptr->compare_value;
		//Output Compare A Match Interrupt Enable
		TIMSK |= (1 << OCIE1A);
	}

}

/*
 * Description:
 * Function to disable the Timer1.
 */

void TIMER1_deInit(void)
{
	// clear the timer
	TCCR1B=0;
	//But still enable overflow interrupts and output compare matches
	TIMSK&=~(1<<TOIE1);
	TIMSK&=~(1<<OCIE1A);

}

/*
 * Description:
 * Function to set the Call Back function address.
 */

void TIMER1_setCallBack(void(*a_ptr)(void))
{
	call_back=a_ptr;
}
