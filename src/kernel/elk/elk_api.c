#include "elk_api.h"
#include "elk_lib.h"

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
uint8_t __elk_verify(uint8_t taskID)
{
    uint8_t i = 0;
    uint8_t flgSearch = 0;      // 0:search interval_start; 0xFF:search interval_end
    uint8_t backupBufIdx = elkBufIdx.idx;
    uint16_t tempNextNodeIdx = elkDualList[backupBufIdx].nextNode;
    ck_set_t tempInterval = _threads[0].task_array[taskID].ck_set;
    for(i=0; i<elkDualList[backupBufIdx].usedNodeNum; i++){
        if((flgSearch == 0) && (tempInterval.start_used_offset < elkListNodes[tempNextNodeIdx].intvlEnd)){
            svIdxIntvlStart = tempNextNodeIdx;
            flgSearch = 0xFF;
        }
        if((flgSearch == 0xFF) && (tempInterval.end_used_offset <= elkListNodes[tempNextNodeIdx].intvlEnd)){
            svIdxIntvlEnd = tempNextNodeIdx;
            break;
        }
        tempNextNodeIdx = elkDualList[backupBufIdx].stElkList[tempNextNodeIdx].nextNode;
        //TODO: print debug info here to show `tempNextNodeIdx`
    }

    uint8_t tempResult = 0;
    uint16_t tempCksum = 0;
    uint8_t tempNodeIdx = svIdxIntvlStart;
    for(i=0; i<elkDualList[backupBufIdx].usedNodeNum; i++){
        if(GET_BIT(svVrfiedBp,tempNodeIdx) == 0){
            tempCksum = _elk_crc(   elkListNodes[tempNodeIdx].intvlStart,   \
                                    elkListNodes[tempNodeIdx].intvlEnd,     \
                                    backupBufIdx);
            if(tempCksum != elkListNodes[tempNodeIdx].subCksum){
                //tempResult = VERIFY_FAILED;           //FIXME:!!!
                //break;             
                SET_BIT(svVrfiedBp,tempNodeIdx);
            }else{
                SET_BIT(svVrfiedBp,tempNodeIdx);
            }
        }
        if(tempNodeIdx == svIdxIntvlEnd){
            tempResult = VERIFY_PASS;
            break;
        }else{
            tempNodeIdx = elkDualList[backupBufIdx].stElkList[tempNodeIdx].nextNode;
        }
    }
    return tempResult;
}

/* --------------------srliu:DONE!!!
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
void __elk_first_cksum()
{
    uint8_t workingBufIdx = elkBufIdx._idx;
    elkNodeBitmaps[workingBufIdx] = 0x0001;                     // use node 0 in elkListNodes[]
    _elk_listFirstAdd(&elkDualList[workingBufIdx], 0);

    elkListNodes[0].intvlStart  = 0;                            // in byte
    elkListNodes[0].intvlEnd    = _threads[0].buffer.size-1;    // in byte
    elkListNodes[0].subCksum    = _elk_crc(elkListNodes[0].intvlStart, elkListNodes[0].intvlEnd, workingBufIdx);

    svVrfiedBp = 0x0001;                                        // update the verified bitmap
}

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
void __elk_checksum(uint8_t taskID)
{
    ck_set_t tempCkSet = _threads[0].task_array[taskID].ck_set;
    _elk_normal_cksum(tempCkSet.start_used_offset, tempCkSet.end_used_offset);
}

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
void __elk_backup(uint8_t taskID)
{
    //global data.  backup-->working
    buffer_t *buffer = &_threads[0].buffer;
    __dma_word_copy((unsigned int)buffer->buf[elkBufIdx.idx],     \
                    (unsigned int)buffer->buf[elkBufIdx._idx],    \
                    (unsigned short)buffer->size>>1);
    
    //Dual-list.    backup-->working
    __dma_word_copy((unsigned int)&elkDualList[elkBufIdx.idx],    \
                    (unsigned int)&elkDualList[elkBufIdx._idx],   \
                    (unsigned short)nvListSize>>1);

    //List nodeBp.  backup-->working
    elkNodeBitmaps[elkBufIdx._idx] = elkNodeBitmaps[elkBufIdx.idx];
}

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
cksum_temp_t __elk_update_nv() 
{
    uint8_t tempI = 0;
    uint8_t tempCntr = 0;
    uint8_t workingBufIdx = elkBufIdx._idx;                 //has change to working buffer
    uint16_t tempN = elkNodeBitmaps[workingBufIdx];
    cksum_temp_t tempCksum;
    tempCksum.svCksumTemp = 0;

    for(tempCntr=0; tempN; ++tempCntr){                     //Find out the number of new nodes.
        tempN &= (tempN-1);
    }

    //HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;          //FIXME:!!!
    while(tempCntr){                                        //Compute total-cksum using sub-cksums.
        if(GET_BIT(elkNodeBitmaps[workingBufIdx],tempI)){
            tempCntr--;
            tempCksum.svCksumTemp ^= elkListNodes[tempI].subCksum;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
        }
        tempI++;
    }

    for(tempI=0;tempI<nvListSize;tempI=tempI+2){            //__elk_du memory
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[workingBufIdx]+tempI);
    }
    tempCksum.nvCksumTemp = HWREG16(CRC32_BASE + OFS_CRCDI);

    return tempCksum;
}

/* ------------------
 * [__elk_ckeck_nv]:
 * LOG: update total cksum to NVM.
 * 1. compute the __shared() cksum;
 * 2. combine it with all elk variables;
 */
uint16_t __elk_check_nv()
{
    uint8_t tempI = 0;
    uint8_t backupBufIdx = elkBufIdx.idx;

    uint16_t tempCksum;

    uint8_t tempCntr = 0;
    volatile uint16_t tempN = elkNodeBitmaps[backupBufIdx];
    for(tempCntr=0; tempN; ++tempCntr){ //find out the number of new nodes.
        tempN &= (tempN-1);
    }

    tempCksum = 0;
    while(tempCntr){
        if(GET_BIT(elkNodeBitmaps[backupBufIdx],tempI)){
            tempCntr--;
            tempCksum ^= elkListNodes[tempI].subCksum;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
        }
        tempI++;
    }

    for(tempI=0;tempI<nvListSize;tempI=tempI+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupBufIdx]+tempI);
        //HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupBufIdx]+tempI+2);
    }

    if(HWREG16(CRC32_BASE + OFS_CRC32INIRESW0)!=nvTotalCksum){
        return VERIFY_PASS;
    }

    if(tempCksum!=elkTotalCksum){
        return VERIFY_PASS;
    }
    return VERIFY_PASS;
}

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
                            cksum_temp_t tempCksum)
{
    elkBufIdx._idx  = elkBufIdx._idx ^ 1;
    elkBufIdx.idx   = elkBufIdx.idx ^ 1;
    elkCurTaskID    = tempTaskID;
    nvTotalCksum    = tempCksum.nvCksumTemp;
    elkTotalCksum   = tempCksum.svCksumTemp;
}

/* -------------
 * [__elk_init]: done!!
 * LOG: ELK variables inited.
 */
void __elk_init() {
    //printk("__elk_init() is called.\r\n");
    nvInited = 0;                   // To un-booted
    elkCurMode = 0;                 // To normal-mode
    elkClearMark = 1;               // To need-clear
    elkCurTaskID = 0;               // To the-first-TASK-ID

    svVrfiedBp = 0;                 // To clear-verified-bitmap, also should clear in power-on ISR.
    svIntervalNum = 0;              // To clear-interval-number, also should clear in power-on ISR.
    svMarkedBp = 0;                 // To clear-marked-bitmap, also should clear in power-on ISR.

    nvTaskNum = 0;
    nvFailedNum++;
    //Dual-list init.
    _elk_listInit(&elkDualList[0]);
    _elk_listInit(&elkDualList[1]);

    //Buffer-Idx init. Backup->0, Working->1
    elkBufIdx.idx = 0;
    elkBufIdx._idx = 1;

    //List-node bitmap init.
    elkNodeBitmaps[0] = 0;
    elkNodeBitmaps[1] = 0;
    elkNodeBitmaps[2] = 0;

    backupSum = 0;
    cksumSum = 0;
    updateSum = 0;
    taskSum = 0;
    markSum = 0;
    verifySum = 0;
}
