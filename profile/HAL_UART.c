//****************************************************************************
//
// HAL_UART.c - Hardware abstraction layer for UART with MSP432P401R
//
//****************************************************************************

#include <driverlib.h>
#include "HAL_UART.h"


/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
EUSCI_A_UART_initParam uartParam =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,                 // SMCLK Clock Source
		8,                                              // BRDIV = 4
        10,                                              // UCxBRF = 5
        247,                                             // UCxBRS = 85
        EUSCI_A_UART_NO_PARITY,                         // No Parity
        EUSCI_A_UART_LSB_FIRST,                         // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,                      // One stop bit
        EUSCI_A_UART_MODE,                              // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION   // Oversampling
};


/* Initializes Backchannel UART GPIO */
void UART_initGPIO()
{
    // Configure P2.0 - UCA0TXD and P2.1 - UCA0RXD
    GPIO_setOutputLowOnPin(UART_TXD_PORT, UART_TXD_PIN);
    GPIO_setAsOutputPin(UART_TXD_PORT, UART_TXD_PIN);
    /* Selecting UART functions for TXD and RXD */
    GPIO_setAsPeripheralModuleFunctionInputPin(
            UART_TXD_PORT,
            UART_TXD_PIN,
			UART_SELECT_FUNCTION);

    GPIO_setAsPeripheralModuleFunctionInputPin(
            UART_RXD_PORT,
            UART_RXD_PIN,
			UART_SELECT_FUNCTION);
}




/***************************************************************************//**
 * @brief  Configures UART
 * @param  none
 * @return none
 ******************************************************************************/

void UART_init(void)
{
    /* Configuring UART Module */
    EUSCI_A_UART_init(EUSCI_A0_BASE, &uartParam);

    /* Enable UART module */
    EUSCI_A_UART_enable(EUSCI_A0_BASE);

	return;
}


/* Transmits String over UART */
void UART_transmitString( char *pStr )
{
	while( *pStr )
	{
		EUSCI_A_UART_transmitData(EUSCI_A0_BASE, *pStr );
		pStr++;
	}
}
