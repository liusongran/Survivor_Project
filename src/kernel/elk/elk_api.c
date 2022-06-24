#include "elk_api.h"
#include "elk_lib.h"


uint8_t __elk_verify(uint8_t taskID)
{
    uint8_t i = 0;
    uint8_t flgSearch = 0;      // 0:search interval_start; 0xFF:search interval_end
    uint8_t dualIdx = elkDualIdx.idx;          //working on working dual idx
    // uint8_t backupBufIdx = elkBufIdx.idx;
    uint16_t tempNextNodeIdx = elkDualList[dualIdx].nextNode;

    ck_set_t tempInterval = _threads[0].task_array[taskID].ck_set;
    for(i=0; i<elkDualList[dualIdx].usedNodeNum; i++){
        if((flgSearch == 0) && (tempInterval.start_verify_offset < elkListNodes[tempNextNodeIdx].intvlEnd)){
            svIdxIntvlStart = tempNextNodeIdx;
            flgSearch = 0xFF;
        }
        if((flgSearch == 0xFF) && (tempInterval.end_verify_offset <= elkListNodes[tempNextNodeIdx].intvlEnd)){
            svIdxIntvlEnd = tempNextNodeIdx;
            break;
        }
        tempNextNodeIdx = elkDualList[dualIdx].stElkList[tempNextNodeIdx].nextNode;
        //TODO: print debug info here to show `tempNextNodeIdx`
    }
    uint8_t tempResult = 0;
    uint16_t tempCksum = 0;
    uint8_t tempNodeIdx = elkDualList[dualIdx].nextNode;
#if (DEBUG_VERIFY==1)
    printf("|!!!1.|verify|bacupBufIdx:%d.\r\n", dualIdx);
    printf("|!!!1.|verify|taskID:%d, workingusedNode:%d.\r\n", taskID, elkDualList[elkBufIdx._idx].usedNodeNum);
    printf("|!!!1.|verify|taskID:%d, backupusedNode:%d.\r\n", taskID, elkDualList[dualIdx].usedNodeNum);
    for(i=0; i<elkDualList[dualIdx].usedNodeNum; i++){
        printf("|!!!1.|verify|nodeIdx:%u, intvS:%u, intvE:%u.\r\n", tempNodeIdx, elkListNodes[tempNodeIdx].intvlStart, elkListNodes[tempNodeIdx].intvlEnd);
        tempNodeIdx = elkDualList[dualIdx].stElkList[tempNodeIdx].nextNode;
    }
#endif

    tempNodeIdx = svIdxIntvlStart;
    for(i=0; i<elkDualList[dualIdx].usedNodeNum; i++){
        if(GET_BIT(svVrfiedBp,tempNodeIdx) == 0){
            tempCksum = _elk_crc(   elkListNodes[tempNodeIdx].intvlStart,   \
                                    elkListNodes[tempNodeIdx].intvlEnd,     \
                                    1);
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
            tempNodeIdx = elkDualList[dualIdx].stElkList[tempNodeIdx].nextNode;
        }
    }
    return tempResult;
}


void __elk_first_cksum()
{
    uint8_t dualIdx = elkDualIdx._idx;
    elkNodeBitmaps[dualIdx] = 0x0001;                     // use node 0 in elkListNodes[]
    _elk_listFirstAdd(&elkDualList[dualIdx], 0);

    elkListNodes[0].intvlStart  = 0;                            // in byte
    elkListNodes[0].intvlEnd    = _threads[0].buffer.size-1;    // in byte
    elkListNodes[0].subCksum    = _elk_crc(elkListNodes[0].intvlStart, elkListNodes[0].intvlEnd, 1);

    svVrfiedBp = 0x0001;                                        // update the verified bitmap
#if (DEBUG_CKSUM == 1)
    printk("|+++4.|cksum|Working Used node:%d.\r\n", elkDualList[dualIdx].usedNodeNum);
    printk("|+++4.|cksum|Backup Used node:%d.\r\n", elkDualList[elkBufIdx.idx].usedNodeNum);
    printk("|+++4.|cksum|Bitmap[0]:%d.Bitmap[1]:%d.Bitmap[2]:%d.\r\n", elkNodeBitmaps[0], elkNodeBitmaps[1], elkNodeBitmaps[2]);
#endif
}


void __elk_checksum(uint8_t taskID)
{
    ck_set_t *tempCkSet = &_threads[0].task_array[taskID].ck_set;
    if(tempCkSet->end_backup_offset){
        _elk_normal_cksum(tempCkSet->start_cksum_offset, tempCkSet->end_cksum_offset);
    }
}


void __elk_backup(uint8_t taskID)
{
#if (DEBUG_BACKUP==1)
    printk("|+++2.|backup|elkDualList[0].usedNodeNum:%d.\r\n",elkDualList[0].usedNodeNum);
    printk("|+++2.|backup|elkDualList[1].usedNodeNum:%d.\r\n",elkDualList[1].usedNodeNum);
    printk("|+++2.|backup|Bitmap[0]:%d.Bitmap[1]:%d.Bitmap[2]:%d.\r\n", elkNodeBitmaps[0],elkNodeBitmaps[1],elkNodeBitmaps[2]);
#endif
    //global data.  backup-->working
    buffer_t *buffer = &_threads[0].buffer;
    // __dma_word_copy((unsigned int)buffer->buf[elkBufIdx.idx],     \
    //                 (unsigned int)buffer->buf[elkBufIdx._idx],    \
    //                 (unsigned short)buffer->size>>1);
    
    // //Dual-list.    backup-->working
    // __dma_word_copy((unsigned int)&elkDualList[elkBufIdx.idx],    \
    //                 (unsigned int)&elkDualList[elkBufIdx._idx],   \
    //                 (unsigned short)nvListSize>>1);

    ck_set_t backupInterval = _threads[0].task_array[taskID].ck_set;

    __dma_word_copy((unsigned int)buffer->buf[elkBufIdx._idx] + backupInterval.start_backup_offset,     \
                    (unsigned int)buffer->buf[elkBufIdx.idx] +  backupInterval.start_backup_offset,    \
                    backupInterval.backup_size >>1);

    //Dual-list.    backup-->working
    __dma_word_copy((unsigned int)&elkDualList[elkDualIdx.idx],    \
                    (unsigned int)&elkDualList[elkDualIdx._idx],   \
                    (unsigned short)nvListSize>>1);

    //List nodeBp.  backup-->working
    // elkNodeBitmaps[elkBufIdx._idx] = elkNodeBitmaps[elkBufIdx.idx];
    elkNodeBitmaps[elkDualIdx._idx] = elkNodeBitmaps[elkDualIdx.idx];
}


cksum_temp_t __elk_update_nv() 
{
    uint8_t tempI = 0;
    uint8_t workingDualIdx = elkDualIdx._idx;                 //always working on working buffer
    uint8_t tempCntr = elkDualList[workingDualIdx].usedNodeNum;//get current used node number

    cksum_temp_t tempCksum;
    tempCksum.svCksumTemp = 0;

    //HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;          //FIXME:!!!
    while(tempCntr){                                        //Compute total-cksum using sub-cksums.
        if(GET_BIT(elkNodeBitmaps[workingDualIdx],tempI)){
            tempCntr--;
            //tempCksum.svCksumTemp ^= elkListNodes[tempI].subCksum;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
        }
        tempI++;
    }

    for(tempI=0;tempI<nvListSize;tempI=tempI+2){            //__elk_du memory
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[workingDualIdx]+tempI);
    }
    tempCksum.nvCksumTemp = HWREG16(CRC32_BASE + OFS_CRCDI);

    return tempCksum;
}


uint16_t __elk_check_nv()
{
    uint8_t tempI = 0;
    uint8_t backupDualIdx = elkDualIdx.idx;

    uint16_t tempCksum;
    /*
    uint8_t tempCntr = 0;
    volatile uint16_t tempN = elkNodeBitmaps[backupBufIdx];
    for(tempCntr=0; tempN; ++tempCntr){ //find out the number of new nodes.
        tempN &= (tempN-1);
    }*/

    uint8_t tempCntr = elkDualList[backupDualIdx].usedNodeNum;

    tempCksum = 0;
    while(tempCntr){
        if(GET_BIT(elkNodeBitmaps[backupDualIdx],tempI)){
            tempCntr--;
            tempCksum ^= elkListNodes[tempI].subCksum;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
        }
        tempI++;
    }

    for(tempI=0;tempI<nvListSize;tempI=tempI+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupDualIdx]+tempI);
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


inline void __elk_commit(   uint8_t tempTaskID, 
                            cksum_temp_t tempCksum)
{
    // elkBufIdx._idx  = elkBufIdx._idx ^ 1;
    // elkBufIdx.idx   = elkBufIdx.idx ^ 1;
    elkDualIdx._idx  = elkDualIdx._idx ^ 1;
    elkDualIdx.idx   = elkDualIdx.idx ^ 1;

    elkCurTaskID    = tempTaskID;
    nvTotalCksum    = tempCksum.nvCksumTemp;
    elkTotalCksum   = tempCksum.svCksumTemp;
}


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

    //DualList-Idx init. Backup->0, Working->1
    elkDualIdx.idx = 0;
    elkDualIdx._idx = 1;

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
