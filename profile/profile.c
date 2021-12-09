#include <msp430.h>
#include "stdio.h"
#include <profile.h>
#include <driverlib.h>
#include <string.h>
#include <HAL_UART.h>


int fputc(int _c, register FILE *_fp)
{
  EUSCI_A_UART_transmitData(EUSCI_A0_BASE, (unsigned char) _c );
  return((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp)
{
  unsigned int i, len;

  len = strlen(_ptr);

  for(i=0 ; i<len ; i++)
  {
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, (unsigned char) _ptr[i]);
  }

  return len;
}
