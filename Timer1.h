/*
 * Module: Timer1
 *
 * File Name: Timer1.h
 *
 * Description: Header file for Timer 1 AVR driver
 *
 * Author: Fady Essam
 *
 */

#ifndef TIMER1_H_
#define TIMER1_H_

#include "std_types.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

// Enumeration for the timer1 prescaler value (to choose the suitable frequency)
typedef enum{
	NORMAL,COMPARE
}Timer1_Mode;

typedef enum{
	No_clock,clk_1,clk_8,clk_64,clk_256,clk_1024,ext_clk_falling_edge,ext_clk_rising_edge
}Timer1_Prescaler;

typedef struct{
uint16 initial_value;
uint16 compare_value; // it will be used in compare mode only.
Timer1_Prescaler prescaler;
Timer1_Mode mode;
}Timer1_ConfigType;

/*
 * Description:
 * Function to initialize the Timer driver
 */

void TIMER1_init(const Timer1_ConfigType * Config_Ptr);

/*
 * Description:
 * Function to disable the Timer1.
 */

void TIMER1_deInit(void);

/*
 * Description:
 * Function to set the Call Back function address.
 */

void TIMER1_setCallBack(void(*a_ptr)(void));

#endif/* Timer1_H_ */
