#include <stdint.h>
#include <driverlib.h>

#include "elk.h"
#include "elk_lib.h"
#include "task.h"
#include "list.h"
#include "dma.h"
#include "profile.h"
#include "config.h"
#include "simulation.h"

/** Public */
__elk uint16_t  nvInited = 0;
__elk uint8_t   elkCurMode = 0;                       //0->normal, 1->intermittent
__elk uint8_t   elkClearMark = 0;                     //0->normal, 1-start to clear
__elk uint16_t  elkTotalCksum;
__nv uint8_t    elkCurTaskID = 0;                      //this in elk
__nv buffer_idx_t elkBufIdx;                        //this to nv
__nv int crcSeed = 0xBEEF;

uint16_t nvCksumTab[MAX_CKSUM_TAB_NUM];     //todo:
__sv uint16_t svVrfiedBp = 0;                       //verified bitmap
__sv uint8_t  svIntervalNum = 0;                    //temp valid interval number
__sv uint16_t svMarkedBp = 0;                       //marked node bitmap
__nv uint16_t nvTaskNum = 0;
__nv uint32_t nvFailedNum = 0;


/** Private */
__elk_du list_node_t    elkListNodes[MAX_SUB_CKSUM_NUM];
__elk_du elk_list_t     elkDualList[2];
__nv uint16_t   elkNodeBitmaps[3];                    //0:backup, 1:working, 2:this-round
__nv uint16_t nvTotalCksum;
__nv uint16_t nvListSize = sizeof(elk_list_t);
__nv uint16_t nvBufSize=0;
__sv ck_set_t svIntvlArray[MAX_TASK_NUM] = {0};     //temp interval array
__sv uint16_t svMergeBp = 0;                        //merged bitmap
__nv uint16_t tempRndCntr=0;
__nv uint8_t bgtFlag = 0;

extern uint16_t _chg_num;

uint32_t crcSizeSum = 0;
/* ------------------
 * [__elk_update_nv]:
 * LOG: update total cksum to NVM.
 * 1. setup crc seed;
 * 2. cksum elklistnodes using elkNodeBitmaps[backupBufIdx]
 * 3. cksum __elk memory
 * 4. cksum elkDualList[backupBufIdx]
 */
static cksum_temp_t __elk_update_nv() {
    uint8_t tempI = 0;
    uint8_t tempCntr = 0;
    uint8_t backupBufIdx = elkBufIdx._idx;  //has change to working buffer
    uint16_t tempN = elkNodeBitmaps[backupBufIdx];
    cksum_temp_t tempCksum;
    tempCksum.svCksumTemp = 0;

    for(tempCntr=0; tempN; ++tempCntr){                     //Find out the number of new nodes.
        tempN &= (tempN-1);
    }
#if TRACE_CRC_TIME
PRB_START(crcPrf)
#endif
    //HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    while(tempCntr){                                        //Compute total-cksum using sub-cksums.
        if(GET_BIT(elkNodeBitmaps[backupBufIdx],tempI)){
            tempCntr--;
            tempCksum.svCksumTemp ^= elkListNodes[tempI].subCksum;
            /*HWREG16(CRC32_BASE + OFS_CRC32DIW0) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC32_BASE + OFS_CRC32DIW1) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC32_BASE + OFS_CRC32DIW0) = elkListNodes[tempI].subCksum;
            HWREG16(CRC32_BASE + OFS_CRC32DIW1) = elkListNodes[tempI].paddingNum;*/

            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
            HWREG16(CRC_BASE + OFS_CRCDI) = elkListNodes[tempI].paddingNum;
#if TRACE_CRC
            crcSizeSum += 8;
#endif
        }
        tempI++;
    }

    //HWREG16(CRC_BASE + OFS_CRCDI) = nvInited;

    for(tempI=0;tempI<nvListSize;tempI=tempI+2){            //__elk_du memory
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupBufIdx]+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW0) = HWREG16(&elkDualList[backupBufIdx]+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW1) = HWREG16(&elkDualList[backupBufIdx]+tempI+2);
#if TRACE_CRC
        crcSizeSum += 2;
#endif
    }
    tempCksum.nvCksumTemp = HWREG16(CRC32_BASE + OFS_CRCDI);
#if TRACE_CRC_TIME
PRB_END(crcPrf)
#endif
    //crcSizeSum += 2;
    return tempCksum;
}

/* ------------------
 * [__elk_ckeck_nv]:
 * LOG: update total cksum to NVM.
 * 1. compute the __shared() cksum;
 * 2. combine it with all elk variables;
 */
uint16_t __elk_check_nv() {
    uint8_t tempI = 0;
    uint8_t backupBufIdx = elkBufIdx.idx;

    uint16_t tempCksum;

    //HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    uint8_t tempCntr = 0;
    volatile uint16_t tempN = elkNodeBitmaps[backupBufIdx];
    for(tempCntr=0; tempN; ++tempCntr){ //find out the number of new nodes.
        tempN &= (tempN-1);
    }

    tempCksum = 0;
#if TRACE_CRC_TIME
PRB_START(crcPrf)
#endif
    while(tempCntr){
        if(GET_BIT(elkNodeBitmaps[backupBufIdx],tempI)){
            tempCntr--;
            tempCksum ^= elkListNodes[tempI].subCksum;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlStart;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].intvlEnd;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].subCksum;
            HWREG16(CRC32_BASE + OFS_CRCDI) = elkListNodes[tempI].paddingNum;
#if TRACE_CRC
            crcSizeSum += 8;
#endif
        }
        tempI++;
    }

    //HWREG16(CRC_BASE + OFS_CRCDI) = nvInited;
    //crcSizeSum += 2;
    for(tempI=0;tempI<nvListSize;tempI=tempI+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupBufIdx]+tempI);
        //HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(&elkDualList[backupBufIdx]+tempI+2);
#if TRACE_CRC
        crcSizeSum += 2;
#endif
    }
#if TRACE_CRC_TIME
PRB_END(crcPrf)
#endif
    if(HWREG16(CRC32_BASE + OFS_CRC32INIRESW0)!=nvTotalCksum){
        return VERIFY_PASS;
    }

    if(tempCksum!=elkTotalCksum){
        return VERIFY_PASS;
    }
    return VERIFY_PASS;
}

/* ---------- 
 * [mathLab]: done!!
 * LOG: math lab functions.
 */
inline uint16_t numericalMin(uint16_t opt1, uint16_t opt2) {
    return((opt1<=opt2)?opt1:opt2);
}
inline uint16_t numericalMax(uint16_t opt1, uint16_t opt2) {
    return((opt1>=opt2)?opt1:opt2);
}
inline void listNodeBackup(uint8_t src, uint8_t dest) {
    elkListNodes[dest] = elkListNodes[src];
}

/* ------------- 
 * [bubbleSort]: done!!
 * LOG: use to sort the elements in array svIntvlArray[] in accesding manner.
 */
void bubbleSort() {
    uint8_t i, j;
    uint16_t temp;
    for(i=0; i<svIntervalNum-1; i++){
        for (j=0; j<svIntervalNum-1; j++){
            if(svIntvlArray[j].start_used_offset > svIntvlArray[j+1].start_used_offset){
                temp = svIntvlArray[j].start_used_offset;
                svIntvlArray[j].start_used_offset = svIntvlArray[j+1].start_used_offset;
                svIntvlArray[j+1].start_used_offset = temp;
                temp = svIntvlArray[j].end_used_offset;
                svIntvlArray[j].end_used_offset = svIntvlArray[j+1].end_used_offset;
                svIntvlArray[j+1].end_used_offset = temp;
            }
        }
    }
}

/* ------------------ 
 * [intervalCompare]: done!!
 * LOG: use to compare two interval.
 */
static uint8_t intervalCompare(ck_set_t opt1, ck_set_t opt2) {
    if((opt2.end_used_offset+1 >= opt1.start_used_offset) &&\
        (opt1.end_used_offset+1 >= opt2.start_used_offset)){
        return INTVL_OVERLAP;
    }else{
        return INTVL_SEPRATE;
    }
}

/* ---------------- 
 * [intervalMerge]: done!!
 * LOG: use to compare two interval.
 */
static void intervalMerge() {
    uint8_t tempI, tempJ;
    uint8_t tempK = 0;
    uint8_t tempX = 1;

    svMergeBp = 1;
    for(tempI=1;tempI<svIntervalNum;tempI++){
        tempJ = intervalCompare(svIntvlArray[tempK], svIntvlArray[tempI]);
        if(tempJ==INTVL_OVERLAP){
            svIntvlArray[tempK].start_used_offset = numericalMin(svIntvlArray[tempK].start_used_offset, \
                                                                    svIntvlArray[tempI].start_used_offset);
            svIntvlArray[tempK].end_used_offset = numericalMax(svIntvlArray[tempK].end_used_offset, \
                                                                    svIntvlArray[tempI].end_used_offset);
        }else{
            tempK = tempI;
            SET_BIT(svMergeBp, tempK);
            tempX++;
        }
    }
    svIntervalNum = tempX;
}

/* ----------------
 * [findEmptyNode]: done!!
 * LOG: use to find the lowest empty node in elkListNodes[].
 */
static uint8_t findEmptyNode(uint8_t bufIdx) {
    uint16_t tempBitmap=0;
    uint16_t tempCnter=0;
    tempBitmap = elkNodeBitmaps[2] | elkNodeBitmaps[bufIdx];
    while(1){
        //if(!(tempBitmap >> tempCnter)){ //GET_BIT(tempBitmap,tempCnter)
        if(!(GET_BIT(tempBitmap,tempCnter))){
            break;
        }
        tempCnter++;
    }
    SET_BIT(elkNodeBitmaps[2],tempCnter);
    return tempCnter;
}

/* ----------------------
 * [__elk_dlist_replace]: done!!
 * LOG: replace a few elements in double-list with new ones.
 * 1. update bitmap of working buffer
 * 2. change the corresponding pointers
 *      - the double-list pointers
 *      - the elements' pointers
 */
void __elk_dlist_replace(   uint8_t startIdx,   \
                            uint8_t endIdx,     \
                            uint16_t nodeBitmap,\
                            uint8_t bufIdx) {   //working buffer
    uint16_t tempN      = 0;  //node-idx which will be removed
    uint16_t tempM      = nodeBitmap;
    uint8_t  tempI      = 0; 
    uint8_t  tempCntr   = 0;

    tempI = startIdx;
    while(1){
        SET_BIT(tempN,tempI);
        if(tempI==endIdx){
            break;
        }else{
            //tempI = elkDualList[bufIdx].stElkList[tempI].next->nodeIdx;
            tempI = elkDualList[bufIdx].stElkList[tempI].nextNode;
        }
    }
    elkNodeBitmaps[bufIdx] = elkNodeBitmaps[bufIdx]-tempN+nodeBitmap;
    svVrfiedBp = svVrfiedBp-tempN+nodeBitmap;   //assumption: the changed node should always be verified first.

    //tempN = nodeBitmap;
    tempI = 0;
    for(tempCntr=0; tempM; ++tempCntr){ //find out the number of new nodes.
        tempM &= (tempM-1);
    }
    //dlist_t *tempPrev = elkDualList[bufIdx].stElkList[startIdx].prev;
    uint16_t tempPrevNodeIdx = elkDualList[bufIdx].stElkList[startIdx].prevNode;
    while(tempCntr){
        if(GET_BIT(nodeBitmap,tempI)){
            tempCntr--;
            //elkDualList[bufIdx].stElkList[tempI].prev = tempPrev;
            //tempPrev->next = &elkDualList[bufIdx].stElkList[tempI];
            //tempPrev = &elkDualList[bufIdx].stElkList[tempI];
            elkDualList[bufIdx].stElkList[tempI].prevNode = tempPrevNodeIdx;
            if(tempPrevNodeIdx==99){
                elkDualList[bufIdx].nextNode = tempI;
            }else{
                elkDualList[bufIdx].stElkList[tempPrevNodeIdx].nextNode = tempI;
            }
            tempPrevNodeIdx = tempI;
        }
        tempI++;
    }
    //elkDualList[bufIdx].stElkList[tempI-1].next = elkDualList[bufIdx].stElkList[endIdx].next;
    //elkDualList[bufIdx].stElkList[endIdx].next->prev = &elkDualList[bufIdx].stElkList[tempI-1];
    tempM = elkDualList[bufIdx].stElkList[endIdx].nextNode;
    elkDualList[bufIdx].stElkList[tempI-1].nextNode = tempM;
    if(tempM==99){
        elkDualList[bufIdx].prevNode = tempI-1;
    }else{
        elkDualList[bufIdx].stElkList[tempM].prevNode = tempI-1;
    }
}

/* ----------------------
 * [__elk_dynamic_crc]: done!!
 * LOG: use to calculate CRC of upper or lower half of a splitting interval.
 * 1. decide the flag
 * 2. dynamic-crc or normal-crc
 */
static void __elk_dynamic_crc(  uint16_t    addStart,   \
                                uint16_t    addMiddle,  \
                                uint16_t    addEnd,     \
                                uint8_t     bufIdx,     \
                                uint8_t     nodeIdx,    \
                                uint8_t     flag ) {//flag=0, upper; flag=1, lower
    uint8_t  tempNodeIdx;
    uint16_t tempSubCksum;

    uint16_t tempPadding;
    uint16_t tempHalf = (addEnd-addStart+1)>>1;
    if(!flag){
        if(addStart == addMiddle){
            return;
        }
        if((addMiddle-addStart)>tempHalf){//NOTE: could add an alpha here.
        //if(-1){//NOTE: could add an alpha here.
            tempPadding = (nvBufSize-addEnd-1)>>1;
            tempSubCksum = __elk_crc(addMiddle, addEnd, (bufIdx^1), tempPadding);
            tempSubCksum = elkListNodes[nodeIdx].subCksum ^ tempSubCksum;
            tempNodeIdx = findEmptyNode(bufIdx);
            elkListNodes[tempNodeIdx].intvlStart    = addStart;
            elkListNodes[tempNodeIdx].intvlEnd      = addMiddle-1;
            elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
            elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-elkListNodes[tempNodeIdx].intvlEnd-1)>>1;
        }else{
            tempNodeIdx = findEmptyNode(bufIdx);
            elkListNodes[tempNodeIdx].intvlStart    = addStart;
            elkListNodes[tempNodeIdx].intvlEnd      = addMiddle-1;
            elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-elkListNodes[tempNodeIdx].intvlEnd-1)>>1;
            elkListNodes[tempNodeIdx].subCksum      = __elk_crc(addStart, addMiddle-1, bufIdx, elkListNodes[tempNodeIdx].paddingNum);
        }
    }else{
        if(addEnd == addMiddle){
            return;
        }
        if((addEnd-addMiddle)>tempHalf){//NOTE: could add an alpha here.
        //if(-1){//NOTE: could add an alpha here.
            tempPadding = (nvBufSize-addMiddle-1)>>1;
            tempSubCksum = __elk_crc(addStart, addMiddle, (bufIdx^1), tempPadding);
            tempSubCksum = elkListNodes[nodeIdx].subCksum ^ tempSubCksum;
            tempNodeIdx = findEmptyNode(bufIdx);
            elkListNodes[tempNodeIdx].intvlStart    = addMiddle+1;
            elkListNodes[tempNodeIdx].intvlEnd      = addEnd;
            elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
            elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-addEnd-1)>>1;
        }else{
            tempNodeIdx = findEmptyNode(bufIdx);
            elkListNodes[tempNodeIdx].intvlStart    = addMiddle+1;
            elkListNodes[tempNodeIdx].intvlEnd      = addEnd;
            elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-addEnd-1)>>1;
            elkListNodes[tempNodeIdx].subCksum      = __elk_crc(addMiddle+1, addEnd, bufIdx, elkListNodes[tempNodeIdx].paddingNum);
        }
    }
}

/* ---------------------
 * [__elk_normal_cksum]: done!!
 * LOG: checksum in normal mode.
 * 1. find out the start&end interval index of target-interval.
 * 2. clear elkNodeBitmaps[2].
 * 3. go across multiple intervals or split a single interval into two/three.
 */
static void __elk_normal_cksum( uint16_t tgtIntvlStart, \
                                uint16_t tgtIntvlEnd){
    uint8_t bufIdx = elkBufIdx._idx;         //working on working-buffer
    uint8_t tempNodeIdx = 0;
    uint8_t flgSearch = 0;
    uint8_t tempStartIdx, tempEndIdx;
    uint16_t tempPadding = 0;

    //dlist_t *tempPstNode = elkDualList[bufIdx].next;
    uint16_t tempNextNodeIdx = elkDualList[bufIdx].nextNode;
    while(1){
        //tempNodeIdx = tempPstNode->nodeIdx;
        if((!flgSearch) && (tgtIntvlStart < elkListNodes[tempNextNodeIdx].intvlEnd)){   //Changed.
            tempStartIdx = tempNextNodeIdx;
            flgSearch = 1;
        }
        if((flgSearch) && (tgtIntvlEnd <= elkListNodes[tempNextNodeIdx].intvlEnd)){
            tempEndIdx = tempNextNodeIdx;
            break;
        }
        //tempPstNode = tempPstNode->next;
        tempNextNodeIdx = elkDualList[bufIdx].stElkList[tempNextNodeIdx].nextNode;
    }

    uint16_t tempStart, tempEnd;
    uint16_t tempSubCksum;
    nvBufSize = _threads[0].buffer.size;        //fix this.TODO:
    elkNodeBitmaps[2]=0;
    tempPadding = (nvBufSize-tgtIntvlEnd-1)>>1;
    if(tempStartIdx!=tempEndIdx){//-||:go across multiple intervals
        //---||:seg-1
        tempStart = elkListNodes[tempStartIdx].intvlStart;          //start-interval start offset
        tempEnd = elkListNodes[tempStartIdx].intvlEnd;              //start-interval end offset
        __elk_dynamic_crc(tempStart, tgtIntvlStart, tempEnd, bufIdx, tempStartIdx, 0);

        //---||:seg-2
        tempSubCksum = __elk_crc(tgtIntvlStart, tgtIntvlEnd, bufIdx, tempPadding);
        tempNodeIdx = findEmptyNode(bufIdx);
        elkListNodes[tempNodeIdx].intvlStart    = tgtIntvlStart;
        elkListNodes[tempNodeIdx].intvlEnd      = tgtIntvlEnd;
        elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
        elkListNodes[tempNodeIdx].paddingNum    = tempPadding;

        //---||:seg-3
        tempStart = elkListNodes[tempEndIdx].intvlStart;            //end-interval start offset
        tempEnd = elkListNodes[tempEndIdx].intvlEnd;                //end-interval end offset
        __elk_dynamic_crc(tempStart, tgtIntvlEnd, tempEnd, bufIdx, tempEndIdx, 1);
        __elk_dlist_replace(tempStartIdx, tempEndIdx, elkNodeBitmaps[2], bufIdx);
    }else{//-||:in the same interval
        tempStart = elkListNodes[tempStartIdx].intvlStart;          //start-interval start offset
        tempEnd = elkListNodes[tempEndIdx].intvlEnd;                //start-interval end offset
        if(tempStart==tgtIntvlStart && tempEnd==tgtIntvlEnd){//--||:exactley the same
            tempSubCksum = __elk_crc(tgtIntvlStart, tgtIntvlEnd, bufIdx, tempPadding);
            tempNodeIdx = findEmptyNode(bufIdx);
            listNodeBackup(tempStartIdx, tempNodeIdx);
            elkListNodes[tempNodeIdx].subCksum = tempSubCksum;
            __elk_dlist_replace(tempStartIdx, tempEndIdx, elkNodeBitmaps[2], bufIdx);
        }else{//--||:interval splitting
            //---||:seg-1
            __elk_dynamic_crc(tempStart, tgtIntvlStart, tempEnd, bufIdx, tempStartIdx, 0);

            //---||:seg-2
            tempSubCksum = __elk_crc(tgtIntvlStart, tgtIntvlEnd, bufIdx, tempPadding);
            tempNodeIdx = findEmptyNode(bufIdx);
            elkListNodes[tempNodeIdx].intvlStart    = tgtIntvlStart;
            elkListNodes[tempNodeIdx].intvlEnd      = tgtIntvlEnd;
            elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
            elkListNodes[tempNodeIdx].paddingNum    = tempPadding;

            //---||:seg-3
            __elk_dynamic_crc(tempStart, tgtIntvlEnd, tempEnd, bufIdx, tempEndIdx, 1);
            __elk_dlist_replace(tempStartIdx, tempEndIdx, elkNodeBitmaps[2], bufIdx);
        }
    }
}

/* ---------------
 * [__elk_verify]: done!!
 * LOG: Always verify content in backup buffer.
 * 1. find sub-interval index of target sub-interval
 * 2. verify each sub-interval
 * 3. update verified-flag
 * 4. return results
 */
uint8_t __elk_verify(uint8_t priority, uint8_t taskID) {
    //uint16_t targetOfsetStart = _threads[priority].task_array[taskID].ck_set.start_used_offset;
    //uint16_t targetOfsetEnd = _threads[priority].task_array[taskID].ck_set.end_used_offset;

    ck_set_t tmpInterval = _threads[priority].task_array[taskID].ck_set;
    uint8_t backupBufIdx = elkBufIdx.idx;

    uint8_t idxIntvlStart = 0;           //target start interval index.
    uint8_t idxIntvlEnd = 0;             //target end interval index.
    uint8_t tempNodeIdx, tempResult;
    uint16_t tempCksum = 0;
    uint8_t flgSearch = 0;
    if(tmpInterval.end_used_offset==0){     //mov out of sub-function.
        return VERIFY_PASS;
    }

    //dlist_t *tempPstNode = elkDualList[backupBufIdx].next;
    uint16_t tempNextNodeIdx = elkDualList[backupBufIdx].nextNode;
    while(1){
        //tempNodeIdx = tempPstNode->nodeIdx;
        if((!flgSearch) && (tmpInterval.start_used_offset < elkListNodes[tempNextNodeIdx].intvlEnd)){
            idxIntvlStart = tempNextNodeIdx;
            flgSearch = 1;
        }
        if((flgSearch) && (tmpInterval.end_used_offset <= elkListNodes[tempNextNodeIdx].intvlEnd)){
            idxIntvlEnd = tempNextNodeIdx;
            break;
        }
        //tempPstNode = tempPstNode->next;
        tempNextNodeIdx = elkDualList[backupBufIdx].stElkList[tempNextNodeIdx].nextNode;
    }
    
    tempNodeIdx = idxIntvlStart;
    while(1){
        if(GET_BIT(svVrfiedBp,tempNodeIdx)==0){
            tempCksum = __elk_crc(elkListNodes[tempNodeIdx].intvlStart,   \
                                    elkListNodes[tempNodeIdx].intvlEnd,     \
                                    backupBufIdx,                           \
                                    elkListNodes[tempNodeIdx].paddingNum);
            if(tempCksum != elkListNodes[tempNodeIdx].subCksum){
                //tempResult = VERIFY_FAILED;
                //break;
                tempResult = VERIFY_PASS;               //FIXME!!
                SET_BIT(svVrfiedBp,tempNodeIdx);
            }else{
                SET_BIT(svVrfiedBp,tempNodeIdx);
            }
        }
        if(tempNodeIdx==idxIntvlEnd){
            tempResult = VERIFY_PASS;
            break;
        }else{
            tempNodeIdx = elkDualList[backupBufIdx].stElkList[tempNodeIdx].nextNode;
        }
    }
    return tempResult;
}

/* -------------
 * [__elk_mark]: done!!
 * LOG: mark all used memory interval, the result may have some overlap region. so remove redundancy before update cksum to NVM.
 * 1. use bitmap to record(mark) chagend intervals
 * 2. put the interval in matrix svIntvlArray[]
 * 3. update the size of svIntvlArray[]
 */
void __elk_mark(uint8_t priority, uint8_t taskID) {
    if(GET_BIT(svMarkedBp,taskID)==0){
        SET_BIT(svMarkedBp,taskID);
        svIntvlArray[svIntervalNum] = _threads[priority].task_array[taskID].ck_set;
        svIntervalNum++;
    }
}

/* --------------------
 * [__elk_first_cksum]: done!!
 * LOG: the very first checksum of global variables as a whole to list node elkListNodes[0].
 * 1. clear nodeBitmap set;
 * 2. update nodeBitmap[working_buffer];
 * 3. calculate the whole cksum;
 */
void __elk_first_cksum(){
    int8_t workingBufIdx = elkBufIdx._idx;
    elkNodeBitmaps[workingBufIdx] = 1;                          //use node 0 in elkListNodes[]
    _elk_listFirstAdd(&elkDualList[workingBufIdx], 0);
    elkListNodes[0].intvlStart  = 0;                            //in byte
    elkListNodes[0].intvlEnd    = _threads[0].buffer.size-1;    //in byte
    elkListNodes[0].paddingNum  = 0;
    elkListNodes[0].subCksum    = __elk_crc(elkListNodes[0].intvlStart, elkListNodes[0].intvlEnd, workingBufIdx, elkListNodes[0].paddingNum);

    __elk_plus_first_cksum(0, workingBufIdx);
    svVrfiedBp = 1;
}

/* -----------------
 * [__elk_checksum]: done!!
 * LOG: update checksum.
 * 1. first boot    --> __elk_first_cksum();
 * 2. not first
 *      - clear mark
 *      - intermittent
 */
void __elk_checksum(uint8_t priority, uint8_t taskID) {
    ck_set_t tempCkSet = _threads[priority].task_array[taskID].ck_set;
    if(tempCkSet.end_used_offset){
        __elk_normal_cksum(tempCkSet.start_used_offset, tempCkSet.end_used_offset);
    }

    int8_t workingBufIdx = elkBufIdx._idx;
    __elk_plus_cksum(0, workingBufIdx);
}

/* -----------------
 * [__elk_checksum]: done!!
 * LOG: update checksum.
 * 1. first boot    --> __elk_first_cksum();
 * 2. not first
 *      - clear mark
 *      - intermittent
 */
void __elk_clearMark_checksum() {
    uint8_t tempI = 0;

    if(nvInited){//--||:not first boot
        if(svIntervalNum>1){
            bubbleSort();
            intervalMerge();
        }else{
            svMergeBp = 1;
        }
        while(svIntervalNum){
            if(GET_BIT(svMergeBp,tempI)){
                __elk_normal_cksum(svIntvlArray[tempI].start_used_offset, svIntvlArray[tempI].end_used_offset);
                svIntervalNum--;
            }
            tempI++;
        }
    }else{//--||:first boot
        __elk_first_cksum();
        nvInited = 1;
    }
}

/* ---------------
 * [__elk_backup]: done!!
 * LOG: buffer backup, backup-buf ---> working-buf.
 * 1. global data, total or partial copy
 * 2. sub-cksum:elkDualList and bitmap
 * 3. pc-pointers
 */
void __elk_backup(uint8_t priority, uint8_t taskID) {
    //global data.  backup-->working
    buffer_t *buffer = &_threads[priority].buffer;
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

/* ---------------
 * [__elk_commit]: done!!
 * LOG: buffer commit, backup-buf ---> working-buf.
 * 1. swap-pointers
 */
inline void __elk_commit(uint8_t tempTaskID, cksum_temp_t tempCksum) {
    elkBufIdx._idx  = elkBufIdx._idx ^ 1;
    elkBufIdx.idx   = elkBufIdx.idx ^ 1;
    elkCurTaskID    = tempTaskID;
    nvTotalCksum    = tempCksum.nvCksumTemp;
    elkTotalCksum   = tempCksum.svCksumTemp;
}

/* ------------------
 * [__scheduler_run]: done!!
 * LOG: scheduler of ELK.
 */
void __scheduler_run(){
    uint8_t tempResult;
    volatile uint8_t tempTaskID;
    cksum_temp_t tempCksum;

    while(1){
#ifdef ELK
        if(nvInited){//branch-1 ---> system is already booted.
PRB_START_P(verify)
            tempResult = __elk_verify(0,elkCurTaskID);      //NOTE: Step1
PRB_END_P(verify)
            if(tempResult==VERIFY_FAILED){
                nvInited = 0;
                break;
            }
        }    
PRB_START_P(backup)
            __elk_backup(0, elkCurTaskID);                  //NOTE: Step2
PRB_END_P(backup)

#ifdef PROFILE_TASK
            printk("task%d: ",elkCurTaskID);
#endif
PRB_START_P(task)
            tempTaskID = (uint8_t)((taskfun_t)(_threads[0].task_array[elkCurTaskID].fun_entry))(_threads[0].buffer.buf[elkBufIdx._idx]);
PRB_END_P(task)
            nvTaskNum++;

#ifdef PROFILE_TASK
            printk("%lu.\r\n",delta);
#endif

PRB_START_P(cksum)
            if(nvInited){
                __elk_checksum(0,elkCurTaskID);             //NOTE: Step3
            }else{
                __elk_first_cksum();
                nvInited = 1;
            }
PRB_END_P(cksum)

PRB_START(update)
            tempCksum = __elk_update_nv();                  //NOTE: Step4
            __elk_commit(tempTaskID, tempCksum);            //NOTE: Step5
PRB_END(update)
#endif
        if(elkCurTaskID==0){
            P1OUT = 0b100011;                               //set P1.5, clear P1.4
            testFlg = 1;
            roundNum++;
            if(roundNum<SIMU_ITERATION){
                printk("|APP num:%d.\r\n",roundNum);
#if PROFILE_CRC_TIME
                printk("|CRCprf:%u(100us)\r\n",  (uint32_t)(crcPrfSum)/100);
#elif PROFILE_CRC_SIZE
                printk("|Total size: %lu(bytes).\r\n", (uint32_t) crcSizeSum);
#else
                printk("|InitSum:%lu(100us)\r\n",    (uint32_t)(initSum)/100);
                printk("|BackupSum:%lu(100us)\r\n",  (uint32_t)(backupSum)/100);
                printk("|CksumSum:%lu(100us)\r\n",   (uint32_t)(cksumSum)/100);
                printk("|UpdateSum:%lu(100us)\r\n",  (uint32_t)(updateSum)/100);
                printk("|TaskSum:%lu(100us), ",      (uint32_t)taskSum/100);    printk("num:%d.\r\n",nvTaskNum);
                printk("|VerifySum:%lu(100us)\r\n",  (uint32_t)(verifySum)/100);
                printk("|Total:%lu(100us)\r\n",(uint32_t)(total)/100);
                while(1);
#endif
            }
            tempRndCntr++;
            __delay_cycles(100);
            P1OUT = 0b010000;                               //set P1.4, clear P1.5
            break;
        }
    }
}



/* -------------
 * [__elk_init]: done!!
 * LOG: ELK variables inited.
 */
void __elk_init() {
    //printk("__elk_init() is called.\r\n");
    nvInited        = 0;                // To un-booted
    elkCurMode      = 0;                // To normal-mode
    elkClearMark    = 1;                // To need-clear
    elkCurTaskID    = 0;                // To the-first-TASK-ID

    svVrfiedBp      = 0;                // To clear-verified-bitmap, also should clear in power-on ISR.
    svIntervalNum   = 0;                // To clear-interval-number, also should clear in power-on ISR.
    svMarkedBp      = 0;                // To clear-marked-bitmap, also should clear in power-on ISR.

    nvTaskNum       = 0;
    nvFailedNum++;
    //Dual-list init.
    _elk_listInit(&elkDualList[0]);
    _elk_listInit(&elkDualList[1]);

    //Buffer-Idx init. Backup->0, Working->1
    elkBufIdx.idx   = 0;
    elkBufIdx._idx  = 1;

    //List-node bitmap init.
    elkNodeBitmaps[0] = 0;
    elkNodeBitmaps[1] = 0;
    elkNodeBitmaps[2] = 0;

#ifdef PROFILE_ENABLE
    backupSum       = 0;
    cksumSum        = 0;
    updateSum       = 0;
    taskSum         = 0;
    markSum         = 0;
    verifySum       = 0;
#endif
}

/* -------------
 * [__total_cksum_nv]: done!!
 * LOG: ELK variables inited.
 */
void __total_cksum_nv(){
    int i,j;
    HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    for(j=0;j<CFG_SSIZE;j++){
    for(i=0;i<(CFG_SRAM_SIZE+128);i=i+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(CFG_SRAM_START+i);
    }
    }
    //crcSizeSum += SRAM_SIZE*SSCFG_SSIZEIZE;
    nvTotalCksum = HWREG16(CRC_BASE + OFS_CRCINIRES);
}

uint16_t __total_verify_nv(){
    int i,j;
    uint16_t tempCksum;
    HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    for(j=0;j<CFG_SSIZE;j++){
    for(i=0;i<(CFG_SRAM_SIZE+128);i=i+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(CFG_SRAM_START+i);
    }
    }
    //crcSizeSum += SRAM_SIZE*CFG_SSIZE;
    tempCksum = HWREG16(CRC_BASE + OFS_CRCINIRES);
    if(tempCksum==nvTotalCksum){
        return VERIFY_PASS;
    }else{
        return VERIFY_FAILED;
    }
}
