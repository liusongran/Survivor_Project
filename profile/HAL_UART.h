
//****************************************************************************
//
// HAL_UART.h - Prototypes of hardware abstraction layer for UART between
//             MSP432P401R and OPT3001
//
//****************************************************************************

#ifndef __HAL_UART_H_
#define __HAL_UART_H_

#define UART_TXD_PORT        GPIO_PORT_P2
#define UART_TXD_PIN         GPIO_PIN0

#define UART_RXD_PORT        GPIO_PORT_P2
#define UART_RXD_PIN         GPIO_PIN1


//#define UART_SELECT_FUNCTION GPIO_PRIMARY_MODULE_FUNCTION

#define UART_SELECT_FUNCTION GPIO_SECONDARY_MODULE_FUNCTION
void UART_initGPIO(void);
void UART_init(void);
void UART_transmitString( char *pStr );

#endif /* __HAL_UART_H_ */
