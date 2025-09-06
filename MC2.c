/*
 ================================================================================================
 Name        : MC2.c
 Author      : Fady Essam
 Description : MC2 Code for Control_ECU
 Date        : Oct 29, 2023
 ================================================================================================
 */

#define ready 0xaa   /* Define a constant for "ready" status */


#include "util/delay.h"          // Include AVR delay header
#include "buzzer.h"             // Include the buzzer library
#include "timer1.h"             // Include the TIMER1 library
#include "uart.h"               // Include the UART library
#include "external_eeprom.h"    // Include the external EEPROM library
#include "Dc_motor.h"            // Include the DC motor library
#include "TWI.h"                // Include the TWI (I2C) library

/*******************************************************************************
 *                         Enumerations                                 *
 *******************************************************************************/

// Enumeration for application commands
typedef enum {
	NO_COMMAND,
	OPEN_DOOR_COMMAND = 0x10,
	CHANGE_PASSWORD_COMMAND = 0x11,
	ALARM_COMMAND = 0x12,
	Compare_Password = 0xaa
} APP_Commands;

// Enumeration for command comparison results
typedef enum {
	NOT_MATCH,
	MATCH
} tfCommands;


/* Global variables */
uint8 Tick = 0;
uint8 receivedpass[6];

/* Configuration structures */
UART_ConfigType UART_ptr;
Timer1_ConfigType TIMER_ptr;
TWI_ConfigType twi_ptr;

/* Global variable for command */
uint8 command;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

// Function to change the password
void changePassword(void) {
	uint8 pass[6] = {0, 0, 0, 0, 0};
	uint8 pass2[6] = {0, 0, 0, 0, 0};

	// Send "ready" signal to start receiving password
	UART_sendByte(ready);
	UART_receiveString(pass);

	// Send "ready" signal to start receiving the second password
	UART_sendByte(ready);
	UART_receiveString(pass2);

	int i = 0;
	for (i = 1; i < 5; i++) {
		if (pass[i] != pass2[i])
			UART_sendByte(NOT_MATCH);
	}
	for (i = 0; i < 5; i++) {
		// Write the new password to EEPROM
		EEPROM_writeByte((100 + i), pass[i]);
		_delay_ms(10);
	}

	// Send success match signal
	UART_sendByte(MATCH);
}

// Function to compare the entered password with the stored password
void ComparePassword(void) {
	uint8 EEPROM_pass[6] = {1, 2, 3, 4, 5};

	// Send Compare_Password command
	UART_sendByte(Compare_Password);

	// Receive the entered password
	UART_receiveString(receivedpass);

	int i = 0;
	for (i = 1; i < 5; i++) {
		/* Read the stored password from EEPROM */
		EEPROM_readByte((100 + i), &EEPROM_pass[i]);
		_delay_ms(10);
	}

	for (i = 1; i < 5; i++) {
		if (EEPROM_pass[i] != receivedpass[i]) {
			/* Send not match signal */
			UART_sendByte(NOT_MATCH);
		}
	}

	// Send match signal
	UART_sendByte(MATCH);
}

// Function to control the door opening process
void OpenDoor(void) {
	if (Tick == 0) {
		// Rotate the motor in clockwise direction
		DcMotor_Rotate(CW, 100);
	} else if (Tick == 5) {
		// Stop the motor */
		DcMotor_Rotate(STOP, 0);
	} else if (Tick == 6) {
		// Rotate the motor in anti-clockwise direction
		DcMotor_Rotate(ACW, 100);
	} else if (Tick == 11) {
		// Stop the motor and de_initialize Timer1
		DcMotor_Rotate(STOP, 0);
		TIMER1_deInit();
		Tick = -1;
	}
	Tick++;
}

int main(void) {
	// UART configuration
	UART_ptr.baud_rate = BAUD_RATE_9600;
	UART_ptr.bit_data = BIT_DATA_8;
	UART_ptr.parity = NONE;
	UART_ptr.stop_bit = STOP_BIT_1;
	UART_init(&UART_ptr);

	// Timer1 configuration
	TIMER_ptr.mode = COMPARE;
	TIMER_ptr.compare_value = 23440; // 3 seconds
	TIMER_ptr.initial_value = 0;
	TIMER_ptr.prescaler = clk_1024;
	TIMER1_setCallBack(OpenDoor);


	// TWI (I2C) configuration
	twi_ptr.address = 0x01;
	twi_ptr.bit_rate = 400000;
	TWI_init(&twi_ptr);

	// Initialize DC motor and buzzer
	DcMotor_Init();
	Buzzer_init();

	while (1) {
		// Receive command from HMI ECU
		command = UART_recieveByte();

		// Execute the received command from HMI ECU
		switch (command) {
		case Compare_Password:
			ComparePassword();
			break;
		case OPEN_DOOR_COMMAND:
			TIMER1_init(&TIMER_ptr);
			OpenDoor(); // to open the door for the first time
			break;
		case CHANGE_PASSWORD_COMMAND:
			changePassword();
			break;
		case ALARM_COMMAND: {
			Buzzer_on();
			uint8 i = 0;
			for (i = 0; i < 60; i++) {
				_delay_ms(1000);
			}
			Buzzer_off();
		}
		break;
		case NO_COMMAND:
			break;
		default:
			; /* do nothing */
		}
	}
}
