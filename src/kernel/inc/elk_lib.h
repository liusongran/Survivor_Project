#include <stdint.h>
#include <driverlib.h>
#include "mcu.h"

#include "task.h"
#include "list.h"
#include "dma.h"
#include "profile.h"

#define VERIFY_PASS         0
#define VERIFY_FAILED       1

#define SET_BIT(num,offset) num |= ((0x01)<<offset)
#define GET_BIT(num,offset) ((num>>offset) & (0x01))

//#define NULL ((void *)0)

extern  buffer_idx_t elkBufIdx;
extern  uint16_t     elkNodeBitmaps[3];
extern  elk_list_t   elkDualList[2];

extern  uint16_t     svVrfiedBp;
extern  list_node_t  elkListNodes[MAX_SUB_CKSUM_NUM];
extern  thread_t     _threads[MAX_THREAD_NUM];

extern  uint8_t     svIdxIntvlStart;        // target start interval index.
extern  uint8_t     svIdxIntvlEnd;          // target end interval index.

/* ----------------
 * [findEmptyNode]: ELK_LIB_fun
 * LOG: use to find the lowest empty node in elkListNodes[].
 *      always working on working buffer.
 * @para.:
 * 
 * @used global vars:
 *      - `elkBufIdx`
 *      - `elkNodeBitmaps`
 *      - `elkDualList`
 */
inline uint8_t findEmptyNode();

/* ----------------------
 * [_elk_dlist_replace_1to1]: ELK_LIB_fun
 * LOG: replace 1 element in double-list with the new one.
 * 1. find node index to be changed
 * 2. update bitmaps: `elkNodeBitmaps` and `svVrfiedBp`
 * 3. update pointers
 *      - the double-list pointers
 *      - the elements' pointers
 * 4. update `elkDualList[bufIdx].usedNodeNum`, no need in this fun
 * @para. :
 *      - bufIdx: always use working buffer
 *      - nodeBitmap: elkNodeBitmaps[2]
 * @used global vars:
 *      - `elkNodeBitmaps`
 *      - `svVrfiedBp`
 *      - `elkDualList`
 */
void _elk_dlist_replace_1to1(   uint8_t startIdx,       \
                                uint16_t nodeBitmap,    \
                                uint8_t bufIdx);

/* ----------------------
 * [_elk_dlist_replace_1tom]: ELK_LIB_fun
 * LOG: replace 1 element in double-list with new ones.
 * 1. find node index to be changed
 * 2. update bitmaps: `elkNodeBitmaps` and `svVrfiedBp`
 * 3. find out the number of new nodes given in `nodeBitmap`
 * 4. update `elkDualList[bufIdx].usedNodeNum`, no need in this fun
 * 5. update pointers
 *      - the double-list pointers
 *      - the elements' pointers
 * @para. :
 *      - bufIdx: always use working buffer
 *      - nodeBitmap: elkNodeBitmaps[2]
 * @used global vars:
 *      - `elkNodeBitmaps`
 *      - `svVrfiedBp`
 *      - `elkDualList`
 */
void _elk_dlist_replace_1tom(   uint8_t startIdx,       \
                                uint16_t nodeBitmap,    \
                                uint8_t bufIdx);

/* ----------------------
 * [_elk_dlist_replace_mtom]: ELK_LIB_fun
 * LOG: replace some elements in double-list with new ones.
 * 1. find node index to be changed
 * 2. update bitmaps: `elkNodeBitmaps` and `svVrfiedBp`
 * 3. find out the number of new nodes given in `nodeBitmap`
 * 4. update `elkDualList[bufIdx].usedNodeNum`, no need in this fun
 * 5. update pointers
 *      - the double-list pointers
 *      - the elements' pointers
 * @para. :
 *      - bufIdx: always use working buffer
 *      - nodeBitmap: elkNodeBitmaps[2]
 * @used global vars:
 *      - `elkNodeBitmaps`
 *      - `svVrfiedBp`
 *      - `elkDualList`
 */
void _elk_dlist_replace_mtom(   uint8_t startIdx,       \
                                uint8_t endIdx,         \
                                uint16_t nodeBitmap,    \
                                uint8_t bufIdx);

/* ----------------------
 * [_elk_dynamic_crc_upper]: ELK_LIB_fun
 * LOG: use to calculate CRC of upper or lower half of a splitting interval.
 * 1. decide the flag
 * 2. dynamic-crc or normal-crc
 * @para.:
 *      - addrStart: start of the original interval
 *      - addrMiddle: start of the new coming interval
 *      - addrEnd: end of the original interval
 *      - nodeIdx: node index of the first new coming interval
 * @used global vars:
 *      - `elkBufIdx`
 *      - `elkListNodes`
 */
void _elk_dynamic_crc_upper(uint16_t    addrStart,   \
                            uint16_t    addrMiddle,  \
                            uint16_t    addrEnd,     \
                            uint8_t     nodeIdx);

/* ----------------------
 * [_elk_dynamic_crc_lower]: ELK_LIB_fun
 * LOG: use to calculate CRC of upper or lower half of a splitting interval.
 * 1. decide the flag
 * 2. dynamic-crc or normal-crc
 * @para.:
 *      - addrStart: start of the original interval
 *      - addrMiddle: end of the new coming interval
 *      - addrEnd: end of the original interval
 *      - nodeIdx: node index of the first new coming interval
 * @used global vars:
 *      - `elkBufIdx`
 */
void _elk_dynamic_crc_lower(uint16_t    addrStart,   \
                            uint16_t    addrMiddle,  \
                            uint16_t    addrEnd,     \
                            uint8_t     nodeIdx);

/* --------------
 * [_elk_crc]: ELK_LIB_fun
 * LOG: memory-range crc.
 * 1. find the real address;
 * 2. compute the corresponding cksum;
 * 3. update the final cksum by padding zeros;
 * @para.:
 *      - offsetStart: offset of iterval start (in Byte)
 *      - offsetEnd: offset of iterval end (in Byte)
 *      - bufIdx: buffer index
 * @used global vars:
 *      - `_threads`
 */
uint16_t _elk_crc(  uint16_t offsetStart,   \
                    uint16_t offsetEnd,     \
                    uint8_t bufIdx);

/* ---------------------
 * [_elk_normal_cksum]: ELK_LIB_fun
 * LOG: checksum in normal mode.
 * 1. find out the start&end interval index of target-interval.
 * 2. clear elkNodeBitmaps[2].
 * 3. go across multiple intervals or split a single interval into two/three.
 * @para.:
 *      - tgtIntvlStart
 *      - tgtIntvlEnd
 * @used global vars:
 *      - `elkNodeBitmaps`
 *      - `elkBufIdx`
 *      - `svIdxIntvlStart`
 *      - `svIdxIntvlEnd`
 *      - `elkListNodes`
 */
void _elk_normal_cksum( uint16_t tgtIntvlStart, \
                        uint16_t tgtIntvlEnd);
