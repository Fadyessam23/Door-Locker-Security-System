/*
 * Module: Buzzer
 *
 * File Name: Buzzer.h
 *
 * Description: Source file for the Buzzer AVR driver
 *
 *  Author: Fady Essam
 *
 * Created on: Nov 01, 2023
 *
 */

#include "buzzer.h"
#include "gpio.h"

/*
 * Description
 * Setup the direction for the buzzer pin as output pin through the GPIO driver.
 * Turn off the buzzer through the GPIO.
 */

void Buzzer_init(void){

	GPIO_setupPinDirection(PORTC_ID, PIN3_ID, PIN_OUTPUT);

	GPIO_writePin(PORTC_ID, PIN3_ID, LOGIC_LOW);
}

/*
 * Description:
 * Function to enable the Buzzer through the GPIO
 */

void Buzzer_on(void){

	GPIO_writePin(PORTC_ID, PIN3_ID, LOGIC_HIGH);
}

/*
 * Description:
 * Function to disable the Buzzer through the GPIO.
 */

void Buzzer_off(void){

	GPIO_writePin(PORTC_ID, PIN3_ID, LOGIC_LOW);
}

