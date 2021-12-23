#include "profile.h"
#include "elk_memory_defines.h"

#ifdef PROFILE_ENABLE
__nv uint16_t backupStart = 0;
__nv uint16_t backupEnd   = 0;
__nv uint64_t backupSum   = 0;

__nv uint16_t updateStart = 0;
__nv uint16_t updateEnd   = 0;
__nv uint64_t updateSum   = 0;

__nv uint16_t cksumStart  = 0;
__nv uint16_t cksumEnd    = 0;
__nv uint64_t cksumSum    = 0;

__nv uint16_t verifyStart = 0;
__nv uint16_t verifyEnd   = 0;
__nv uint64_t verifySum   = 0;

__nv uint16_t markStart   = 0;
__nv uint16_t markEnd     = 0;
__nv uint64_t markSum     = 0;

__nv uint16_t taskStart   = 0;
__nv uint16_t taskEnd     = 0;
__nv uint64_t taskSum     = 0;

__nv uint16_t initStart   = 0;
__nv uint16_t initEnd     = 0;
__nv uint64_t initSum     = 0;

__nv uint64_t total       = 0;
__nv int64_t _chg_curBgt  = 0;
__nv uint16_t roundNum    = 0;
__nv uint8_t testFlg      = 0;

__nv uint8_t fail_flag    = 0;
__nv int16_t calbriBgt    = 0;
__nv int32_t checkPeriod  = 0;
#endif

int fputc(int _c, register FILE *_fp){
  EUSCI_A_UART_transmitData(EUSCI_A0_BASE, (unsigned char) _c );
  return((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp){
  unsigned int i, len;

  len = strlen(_ptr);
  for(i=0 ; i<len ; i++){
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, (unsigned char) _ptr[i]);
  }
  return len;
}


/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
EUSCI_A_UART_initParam uartParam = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,                 // SMCLK Clock Source
	8,                                              // BRDIV = 4
    10,                                             // UCxBRF = 5
    247,                                            // UCxBRS = 85
    EUSCI_A_UART_NO_PARITY,                         // No Parity
    EUSCI_A_UART_LSB_FIRST,                         // LSB First
    EUSCI_A_UART_ONE_STOP_BIT,                      // One stop bit
    EUSCI_A_UART_MODE,                              // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION   // Oversampling
};

/* Initializes Backchannel UART GPIO */
void __gpio_init(){
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

void __uart_init(void) {
    /* Configuring UART Module */
    EUSCI_A_UART_init(EUSCI_A0_BASE, &uartParam);

    /* Enable UART module */
    EUSCI_A_UART_enable(EUSCI_A0_BASE);

	return;
}

/* Transmits String over UART */
void UART_transmitString( char *pStr ){
	while( *pStr ) {
		EUSCI_A_UART_transmitData(EUSCI_A0_BASE, *pStr );
		pStr++;
	}
}
