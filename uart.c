/******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.c
 *
 * Description: Source file for the UART AVR driver
 *
 * Author: Fady Essam
 *
 *******************************************************************************/

#include "uart.h"
#include "avr/io.h" /* To use the UART Registers */
#include "common_macros.h" /* To use the macros like SET_BIT */

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType *Config_Ptr) {
	// Calculate the UBRR value based on the selected baud rate
	uint16_t ubrr_value ;
	switch (Config_Ptr->baud_rate) {
	case BAUD_RATE_9600:
		ubrr_value = F_CPU / ((uint32)16 * 9600) - 1;
		break;
	case BAUD_RATE_115200:
		ubrr_value = F_CPU / (uint32)((uint32)16 * 115200) - 1;
		break;
	default:
		// Handle unsupported baud rate
		return;
	}
	// Set the baud rate registers
	UBRRH = (uint8)(ubrr_value >> 8);
	UBRRL = (uint8)ubrr_value;

	// Configure data frame format: bit data, parity, and stop bits
	uint8_t data_bits = (Config_Ptr->bit_data);
	uint8_t parity_bit = (Config_Ptr->parity) << UPM0;
	uint8_t stop_bits = (Config_Ptr->stop_bit) << USBS;

	UCSRC = (1 << URSEL) | (parity_bit) | (stop_bits) | (data_bits << UCSZ0);

	// Enable the transmitter and receiver
	UCSRB = (1 << TXEN) | (1 << RXEN);
	if(Config_Ptr->bit_data==BIT_DATA_9)
	{
		UCSRB|=(1<<UCSZ2);
		UCSRC|=(3<< UCSZ0);
	}
}

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data)
{
	/*
	 * UDRE flag is set when the Tx buffer (UDR) is empty and ready for
	 * transmitting a new byte so wait until this flag is set to one
	 */
	while(BIT_IS_CLEAR(UCSRA,UDRE)){}

	/*
	 * Put the required data in the UDR register and it also clear the UDRE flag as
	 * the UDR register is not empty now
	 */
	UDR = data;

	/************************* Another Method *************************
	UDR = data;
	while(BIT_IS_CLEAR(UCSRA,TXC)){} // Wait until the transmission is complete TXC = 1
	SET_BIT(UCSRA,TXC); // Clear the TXC flag
	 *******************************************************************/
}

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void)
{
	/* RXC flag is set when the UART receive data so wait until this flag is set to one */
	while(BIT_IS_CLEAR(UCSRA,RXC)){}

	/*
	 * Read the received data from the Rx buffer (UDR)
	 * The RXC flag will be cleared after read the data
	 */
	return UDR;
}

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str)
{
	uint8 i = 0;

	/* Send the whole string */
	while(Str[i] != '\0')
	{
		UART_sendByte(Str[i]);
		i++;
	}

	/************************* Another Method *************************
	while(*Str != '\0')
	{
		UART_sendByte(*Str);
		Str++;
	}		
	 *******************************************************************/
}

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str)
{
	uint8 i = 0;

	/* Receive the first byte */
	Str[i] = UART_recieveByte();

	/* Receive the whole string until the '#' */
	while(Str[i] != '#')
	{
		i++;
		Str[i] = UART_recieveByte();
	}

	/* After receiving the whole string plus the '#', replace the '#' with '\0' */
	Str[i] = '\0';
}
