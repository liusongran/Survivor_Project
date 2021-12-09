

#ifndef PROFILE_PROFILE_H_TEMP_TEMP_
#define PROFILE_PROFILE_H_TEMP_LE_H_TEMP_

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <driverlib.h>
#include <HAL_UART.h>
#include <string.h>
//#include <printk.h.temp>
//#define PRINT_LOG
#include <elk.h>

#define FAIL_FREQ               2000000000
#define TRACE_CRC               0
#define TRACE_CRC_TIME          0

#define FAIL_FRQ                (FAIL_FREQ)              //in us
#define CKSUM_FRQ               (FAIL_FRQ>>1)               //For total recall

#define SSIZE                   (512)
#define SRAM_SIZE               512


#define TEST_PERIOD             200                  //in seconds
#define FAIL_NUM                (TEST_PERIOD*1000000/FAIL_FRQ)


#define ON_PERIOD               (FAIL_FRQ)     //us
#define WARNING                 (FAIL_FRQ-1000)      //us

//#define TRACE


#ifdef TRACE
#define ITER        3
#else
#define ITER        50
#endif


#define ELK

//#define TOTAL

#define SRAM_START              0x1C00
#define SRAM_END                0x2C00

#define printk      printf

#ifdef DEBUG
#define PRB_START(var)    \
        Timer_A_startCounter( TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE );\
        var##Start = Timer_A_getCounterValue(TIMER_A1_BASE);
#else
#define PRB_START(var)
#endif


#ifdef DEBUG
#define PRB_END(var)    \
        var##End = Timer_A_getCounterValue(TIMER_A1_BASE);\
        Timer_A_stop(TIMER_A1_BASE);\
        Timer_A_clear(TIMER_A1_BASE);\
        delta = (var##End-var##Start)>>2;\
        var##Sum += delta;\
        //printk("[%s]:%d(us)\r\n",#var,delta);
#else
#define PRB_END(var)
#endif


int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);


#endif
