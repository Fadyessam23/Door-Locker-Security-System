/*
 ================================================================================================
 Name        : MC1.c
 Author      : Fady Essam
 Description : MC1 Code for HMI_ECU (Human Machine Interface)
 Date        : Oct 29, 2023
 ================================================================================================
 */


#include <util/delay.h>  // Include AVR delay header
#include "keypad.h"   // Include the keypad library
#include "lcd.h"      // Include the LCD library
#include "timer1.h"   // Include the TIMER1 library
#include "uart.h"     // Include the UART library

#define ready 0xaa    // Define a constant for "ready" command


/*******************************************************************************
 *                      Types Definitions and Enumerations                     *
 *******************************************************************************/

// Enumeration for command comparison results
typedef enum {
	NOT_MATCH,
	MATCH
} tfCommands;

// Enumeration for different application commands
typedef enum {
	NO_COMMAND,
	OPEN_DOOR_COMMAND = 0x10,
	CHANGE_PASSWORD_COMMAND = 0x11,
	ALARM_COMMAND = 0x12,
	Compare_Password = 0xaa
} APP_Commands;


/*******************************************************************************
 *                             Global Variables                                *
 *******************************************************************************/

uint8 comm = 0;
UART_ConfigType Config_Ptr;
Timer1_ConfigType TIMER_ptr;
// Global variable to track time intervals
uint8 Tick = 0;
uint8 pass[6];
uint8 pass2[6];
uint8 pass3[6];


/*******************************************************************************
 *                             Functions Definitions                           *
 *******************************************************************************/


// Function to handle Changing the password
void Step1(void) {
	// Send CHANGE_PASSWORD_COMMAND to initiate password change
	UART_sendByte(CHANGE_PASSWORD_COMMAND);

	// Clear the LCD screen
	LCD_clearScreen();

	// Display message for user to enter the new password
	LCD_displayString("Plz Enter Pass:");

	// Wait until a valid key is pressed (not '=' or 17)
	while ((KEYPAD_getPressedKey() == '=' )| (KEYPAD_getPressedKey() == 17));

	int i = 0;
	pass[5] = '\0';

	// Read and display the password with '*' masking
	for (i = 0; i < 5; i++) {
		pass[i] = KEYPAD_getPressedKey();
		LCD_displayStringRowColumn(1, i, "*");
		_delay_ms(500);
	}
	pass[5] = '#';

	// Wait until the '=' key is pressed
	while (KEYPAD_getPressedKey() != '=');

	// Clear the LCD screen
	LCD_clearScreen();

	// Display message for user to re-enter the same password
	LCD_displayString("Plz Re-enter The");
	LCD_moveCursor(1,0);
	LCD_displayString("Same Pass:");

	_delay_ms(500);

	// Read and display the second password entry with '*' masking
	for (i = 0; i < 5; i++) {
		pass2[i] = KEYPAD_getPressedKey();
		LCD_displayStringRowColumn(1, i+11 , "*");
		_delay_ms(500);
	}
	pass2[5] = '#';

	// Wait until the '=' key is pressed
	while (KEYPAD_getPressedKey() != '=');

	// Send the first password to the receiver (CONTROL ECU)
	sendPassword1();
}

// Function to send the first password to the receiver
void sendPassword1(void) {

	// Send CHANGE_PASSWORD_COMMAND to receiver
	UART_sendByte(CHANGE_PASSWORD_COMMAND);

	// Wait until the receiver is ready
	while (UART_recieveByte() != ready);

	// Send the second password to receiver
	UART_sendString(pass2);

	// Proceed to send the second part of the password
	sendPassword2();
}

// Function to send the second part of the password and compare
void sendPassword2(void) {
	uint8 received_compare_result;

	// Wait until the receiver is ready
	while (UART_recieveByte() != ready);

	// Send the first password to compare
	UART_sendString(pass);

	// Get the received password compare result from UART
	received_compare_result = UART_recieveByte();

	// If the two entered passwords do not match
	if (received_compare_result == NOT_MATCH) {
		// Display error message and retry Step1
		LCD_clearScreen();
		LCD_displayString("NOT MATCH");
		_delay_ms(1000);
		Step1();
	} else if (received_compare_result == MATCH) {
		// Display success message
		LCD_clearScreen();
		LCD_displayString("MATCH");
		_delay_ms(1000);
		return;
	}
}

/* Function to compare the entered password with the stored password */
void comparepassword(void) {
	// Send Compare_Password command to initiate password comparison
	UART_sendByte(Compare_Password);

	// Wait until the receiver is ready for password comparison
	while (UART_recieveByte() != Compare_Password);

	// Send the third password to compare
	UART_sendString(pass3);
}

// Function to perform matching the password with the saved one
uint8 step2(void) {
	uint8 count = 0;
	uint8 compare_value;

	// Loop for a maximum of 3 attempts
	while (count < 3) {
		LCD_clearScreen();
		LCD_displayString("Plz Enter Pass:");
		int i = 0;

		// Wait until valid key is pressed (not '+', '-', '=' or 17)
		while ((KEYPAD_getPressedKey() == '+' )|(KEYPAD_getPressedKey() == '-' )|( KEYPAD_getPressedKey() == '=' )|( KEYPAD_getPressedKey() == 17));

		// Read and display the third password entry with '*' masking
		for (i = 0; i < 5; i++) {
			pass3[i] = KEYPAD_getPressedKey();
			LCD_displayStringRowColumn(1, i, "*");
			_delay_ms(500);
		}
		pass3[5] = '#';

		// Wait until the '=' key is pressed
		while (KEYPAD_getPressedKey() != '=');

		// Compare the entered password with the stored one
		comparepassword();

		// Receive the comparison result from UART
		compare_value = UART_recieveByte();

		// If the entered password matches
		if (compare_value == MATCH) {
			return MATCH;
		}
		count++;
	}

	// If the password entries exceeded the limit, send an ALARM_COMMAND
	UART_sendByte(ALARM_COMMAND);
	LCD_clearScreen();
	LCD_displayString("ERROR");
	// Delay one min error */
	uint8 i = 0;
	for (i = 0; i < 60; i++) {
		_delay_ms(1000);
	}
	return NOT_MATCH;
}

// Function to Handle the motor open and close state
void step3(void) {

	if (Tick == 0) {
		LCD_clearScreen();
		LCD_displayString("Door is Unlocking");
	} else if (Tick == 5) {
		LCD_clearScreen();
		LCD_displayString("Door is Unlocked");
	} else if (Tick == 6) {
		LCD_clearScreen();
		LCD_displayString("Door is Locking");
	} else if (Tick == 11) {
		/* De_initialize Timer1 and reset Tick */
		TIMER1_deInit();
		Tick = -1;
	}
	Tick++;
}

int main(void) {
	// Timer1 configuration
	TIMER_ptr.mode = COMPARE;
	TIMER_ptr.compare_value = 23440; // 3 seconds
	TIMER_ptr.initial_value = 0;
	TIMER_ptr.prescaler = clk_1024;
	TIMER1_setCallBack(step3);

	// UART configuration
	Config_Ptr.baud_rate = BAUD_RATE_9600;
	Config_Ptr.bit_data = BIT_DATA_8;
	Config_Ptr.parity = NONE;
	Config_Ptr.stop_bit = STOP_BIT_1;
	UART_init(&Config_Ptr);

	// Initialize LCD and execute Step1
	LCD_init();
	Step1();

	while (1) {
		LCD_clearScreen();
		LCD_displayString("+ : OPEN DOOR");
		LCD_displayStringRowColumn(1, 0, "- : CHANGE PASS ");

		// Get user's command from the keypad
		comm = KEYPAD_getPressedKey();

		if (comm == '+') {
			if (step2() == MATCH) {
				// Send OPEN_DOOR_COMMAND to control ECU
				UART_sendByte(OPEN_DOOR_COMMAND);
				TIMER1_init(&TIMER_ptr);
				step3();

				// Wait until Tick is reset
				while (Tick != 0);
			}
		} else if (comm == '-') {
			if (step2() == MATCH) {
				LCD_clearScreen();
				LCD_displayString("CHANGE PASS");
				_delay_ms(1000);
				Step1();
			}
		} else {
			/* Do nothing for other keys */
		}
	}
}
