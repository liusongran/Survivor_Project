#ifndef ELK_API_H_
#define ELK_API_H_
#include <stdint.h>
#include <driverlib.h>
#include "elk_lib.h"

#define DEBUG

extern  uint16_t        nvInited;
extern  buffer_idx_t    elkBufIdx;
extern  uint16_t        elkNodeBitmaps[3];
extern  elk_list_t      elkDualList[2];
extern  uint16_t        nvTaskNum;

extern  uint16_t        svVrfiedBp;
extern  list_node_t     elkListNodes[MAX_SUB_CKSUM_NUM];
extern  thread_t        _threads[MAX_THREAD_NUM];
extern  uint16_t        nvListSize;

extern  uint8_t         svIdxIntvlStart;           // target start interval index.
extern  uint8_t         svIdxIntvlEnd;             // target end interval index.
extern  uint16_t        elkTotalCksum;
extern  uint8_t         elkCurTaskID; 
extern  uint16_t        nvTotalCksum;
extern  uint8_t         elkCurMode;
extern  uint8_t         elkClearMark;
extern  uint8_t         svIntervalNum;
extern  uint16_t        svMarkedBp;
extern  uint32_t        nvFailedNum;

extern  uint64_t        verifySum;
extern  uint64_t        backupSum;
extern  uint64_t        taskSum;
extern  uint64_t        cksumSum;
extern  uint64_t        updateSum;
extern  uint64_t        crcPrfSum;
extern  uint64_t        initSum;
extern  uint64_t        markSum;

/* ---------------
 * [__elk_verify]: ELK_API_fun
 * LOG: Always verify content in backup buffer.
 * 1. find sub-interval index of target sub-interval
 * 2. verify each sub-interval
 * 3. update verified-flag
 * 4. return results
 * @para.:
 *      - taskID
 * @used global vars:
 *      - `_threads`
 *      - `elkBufIdx`
 *      - `svVrfiedBp`
 *      - `elkListNodes`
 *      - `elkDualList`
 *      - `svIdxIntvlStart`
 *      - `svIdxIntvlEnd`
 */
uint8_t __elk_verify(uint8_t taskID);

/* --------------------
 * [__elk_first_cksum]: 
 * LOG: the very first checksum of global variables as a whole to list node elkListNodes[0].
 * 1. clear nodeBitmap set;
 * 2. update nodeBitmap[working_buffer];
 * 3. calculate the whole cksum;
 * @para.:
 * 
 * @used global vars:
 *      - `_threads`
 *      - `elkNodeBitmaps`
 *      - `elkDualList`
 *      - `elkBufIdx`
 *      - `elkListNodes`
 *      - `svVrfiedBp`
 */
void __elk_first_cksum();

/* -----------------
 * [__elk_checksum]:
 * LOG: update checksum.
 * 1. first boot    --> __elk_first_cksum();
 * 2. not first
 *      - clear mark
 *      - intermittent
 * @para.:
 *      - taskID
 * @used global vars:
 *      - `_threads`
 */
void __elk_checksum(uint8_t taskID);

/* ---------------
 * [__elk_backup]: done!!
 * LOG: buffer backup, backup-buf ---> working-buf.
 * 1. global data, total or partial copy
 * 2. sub-cksum:elkDualList and bitmap
 * 3. pc-pointers
 * @para.:
 *      - taskID
 * @used global vars:
 *      - `_threads`
 *      - `nvListSize`
 *      - `elkNodeBitmaps`
 *      - `elkDualList`
 */
void __elk_backup(uint8_t taskID);

/* ------------------
 * [__elk_update_nv]:
 * LOG: update total cksum to NVM.
 * 1. setup crc seed;
 * 2. cksum elklistnodes using elkNodeBitmaps[backupBufIdx]
 * 3. cksum __elk memory
 * 4. cksum elkDualList[backupBufIdx]
 * @para.:
 * 
 * @used global vars:
 *      - `elkBufIdx`
 *      - `nvListSize`
 *      - `elkNodeBitmaps`
 *      - `elkDualList`
 */
cksum_temp_t __elk_update_nv();

/* ------------------
 * [__elk_ckeck_nv]:
 * LOG: update total cksum to NVM.
 * 1. compute the __shared() cksum;
 * 2. combine it with all elk variables;
 */
uint16_t __elk_check_nv();

/* ---------------
 * [__elk_commit]: done!!
 * LOG: buffer commit, backup-buf ---> working-buf.
 * 1. swap-pointers
 * @para.:
 *      - tempTaskID
 *      - tempCksum
 * @used global vars:
 *      - `elkBufIdx`
 *      - `elkCurTaskID`
 *      - `nvTotalCksum`
 *      - `elkTotalCksum`
 */
inline void __elk_commit(   uint8_t tempTaskID, 
                            cksum_temp_t tempCksum);

/* -------------
 * [__elk_init]: done!!
 * LOG: ELK variables inited.
 */
void __elk_init();

void __scheduler_run();

/**
 * TODO:
 * 
 */

void __elk_crc_table_gen();
uint16_t __total_verify_nv();
void __total_cksum_nv();

#endif
