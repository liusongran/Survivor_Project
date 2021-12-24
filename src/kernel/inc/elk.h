#include <stdint.h>
#include "elk_memory_defines.h"

#define DEBUG

#define VERIFY_PASS         0
#define VERIFY_FAILED       1

#define INTVL_OVERLAP       0
#define INTVL_SEPRATE       1

#define NEW_NODE_ADDED      1

#define SET_BIT(num,offset) num |= ((0x01)<<offset)
#define GET_BIT(num,offset) ((num>>offset) & (0x01))

#define ENTER_CRITICAL          __no_operation();\
                                __bic_SR_register(GIE);\
                                __no_operation();

#define EXIT_CRITICAL           __no_operation();\
                                __bis_SR_register(GIE);\
                                __no_operation();

//#define NULL ((void *)0)

extern uint16_t nvInited;
extern uint8_t  elkCurMode;
extern uint8_t  elkClearMark;


void __elk_backup(uint8_t priority, uint8_t taskID);
void __elk_checksum(uint8_t priority, uint8_t taskID);
void __elk_clearMark_checksum();
void __elk_first_cksum();
uint16_t __elk_check_nv();
//void __elk_commit(uint8_t tempTaskID);
void __elk_mark(uint8_t priority, uint8_t taskID);
void __scheduler_run();
void __elk_init();
uint8_t __elk_verify(uint8_t priority, uint8_t taskID);
void __elk_crc_table_gen();



uint16_t __total_verify_nv();
void __total_cksum_nv();

/**
 * ELK_PLUS API
 */

void __elk_plus_init_access();
void __elk_plus_init_mem(uint16_t wholeStartAddr, uint16_t wholeSize);
bool __elk_plus_verify(uint16_t idxI, uint16_t idxJ);
void __elk_plus_cksum();
