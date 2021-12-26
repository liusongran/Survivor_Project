/** 
 * Profile related macros defined here. 
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <msp430.h>
#include <driverlib.h>


/** Debug information log */
#define DEBUG_ENABLE
#define PROFILE_ENABLE
#define PROFILE_CRC_SIZE        0
#define PROFILE_CRC_TIME        0
#define PROFILE_TASK


/** Probes definition */
#ifdef PROFILE_ENABLE
#define PRB_START(var)                                                  \
        Timer_A_startCounter( TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE ); \
        var##Start = Timer_A_getCounterValue(TIMER_A1_BASE);

#define PRB_START_P(var)        PRB_START(var)    
#else
#define PRB_START(var)
#define PRB_START_P(var)
#endif

#ifdef PROFILE_ENABLE
#define PRB_END(var)                                            \
        var##End = Timer_A_getCounterValue(TIMER_A1_BASE);      \
        Timer_A_stop(TIMER_A1_BASE);                            \
        Timer_A_clear(TIMER_A1_BASE);                           \
        delta = (var##End-var##Start)>>2;                       \
        var##Sum += delta;                                      \
        if(_chg_curBgt<delta){                                  \
                total += _chg_curBgt;                           \
                _chg_curBgt = 0;                                \
                break;                                          \
        }else{                                                  \
                total += delta;                                 \
                _chg_curBgt -= delta;                           \
        }
        //printk("[%s]:%d(us)\r\n",#var,delta);

#define PRB_END_P(var)                                          \
        var##End = Timer_A_getCounterValue(TIMER_A1_BASE);      \
        Timer_A_stop(TIMER_A1_BASE);                            \
        Timer_A_clear(TIMER_A1_BASE);                           \
        delta = (var##End-var##Start)>>2;                       \
        var##Sum += delta;                                      \
        if(_chg_curBgt<delta){                                  \
                total += _chg_curBgt;                           \
                var##Sum = var##Sum-delta+_chg_curBgt;          \
                _chg_curBgt = 0;                                \
                break;                                          \
        }else{                                                  \
                total += delta;                                 \
                _chg_curBgt -= delta;                           \
        }
#else
#define PRB_END(var)
#define PRB_END_P(var)  
#endif


/** printf re-define */
#define printk      printf

int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);

/** uart init. */
#define UART_TXD_PORT        GPIO_PORT_P2
#define UART_TXD_PIN         GPIO_PIN0

#define UART_RXD_PORT        GPIO_PORT_P2
#define UART_RXD_PIN         GPIO_PIN1

//#define UART_SELECT_FUNCTION GPIO_PRIMARY_MODULE_FUNCTION
#define UART_SELECT_FUNCTION GPIO_SECONDARY_MODULE_FUNCTION

void __gpio_init(void);
void __uart_init(void);
void UART_transmitString( char *pStr );

extern uint16_t backupStart ;
extern uint16_t backupEnd   ;
extern uint64_t backupSum   ;

extern uint16_t updateStart ;
extern uint16_t updateEnd   ;
extern uint64_t updateSum   ;

extern uint16_t cksumStart  ;
extern uint16_t cksumEnd    ;
extern uint64_t cksumSum    ;

extern uint16_t verifyStart ;
extern uint16_t verifyEnd   ;
extern uint64_t verifySum   ;

extern uint16_t markStart   ;
extern uint16_t markEnd     ;
extern uint64_t markSum     ;

extern uint16_t taskStart   ;
extern uint16_t taskEnd     ;
extern uint64_t taskSum     ;

extern uint16_t initStart   ;
extern uint16_t initEnd     ;
extern uint64_t initSum     ;

extern uint64_t total       ;
extern uint32_t delta       ;
extern int64_t _chg_curBgt  ;
extern uint16_t roundNum    ;
extern uint8_t testFlg      ;

extern uint8_t fail_flag    ;
extern int16_t calbriBgt    ;
extern int32_t checkPeriod  ;
