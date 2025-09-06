/*
 * Module: DC_Motor
 *
 * File Name: DC_Motor.c
 *
 * Description: Source file for the DC_Motor AVR driver
 *
 *  Created on: Oct 16, 2023
 *
 *   Author: Fady Essam
 */

#include "gpio.h"
#include "DC_motor.h"
#include "PWM.h"

/*
 * Description:
 * The Function responsible for setup the direction for the two motor pins through the GPIO driver.
 * Stop at the DC-Motor at the beginning through the GPIO driver.
 */

void DcMotor_Init(void){
	GPIO_setupPinDirection(DC_MOTOR_PORT_ID,DC_MOTOR_PIN0_ID,PIN_OUTPUT);
	GPIO_setupPinDirection(DC_MOTOR_PORT_ID,DC_MOTOR_PIN1_ID,PIN_OUTPUT);
	GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN0_ID,LOGIC_LOW);
	GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN1_ID,LOGIC_LOW);
}

/*
 * Description:
 * The function responsible for rotate the DC Motor CW/ or A-CW
 * or stop the motor based on the state input state value.
 * Send the required duty cycle to the PWM driver based on the required speed value.
 */

void DcMotor_Rotate(DcMotor_State state,uint8 speed){
	PWM_Timer0_Start((uint8)((float32)(speed*255)/100));
	switch(state){

	case STOP:
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN0_ID,LOGIC_LOW);
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN1_ID,LOGIC_LOW);
		PWM_Timer0_Start(speed);
		break;

	case CW:
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN0_ID,LOGIC_LOW);
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN1_ID,LOGIC_HIGH);
		PWM_Timer0_Start(speed);
		break;

	case ACW:
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN0_ID,LOGIC_HIGH);
		GPIO_writePin(DC_MOTOR_PORT_ID,DC_MOTOR_PIN1_ID,LOGIC_LOW);
		PWM_Timer0_Start(speed);
		break;
	}

}
