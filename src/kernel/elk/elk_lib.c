#include "elk_lib.h"

inline void listNodeBackup(uint8_t src, uint8_t dest) {
    elkListNodes[dest] = elkListNodes[src];
}

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
inline uint8_t findEmptyNode()
{
    int i = 0;
    uint16_t tempCnter = 0;
    uint8_t workingBufIdx = elkBufIdx._idx;
    uint16_t tempBitmap = elkNodeBitmaps[2] | elkNodeBitmaps[workingBufIdx];

    for (i=0; i<16; i++){
        if(!(GET_BIT(tempBitmap,tempCnter))){
            break;
        }
        tempCnter++;
    }
    //TODO: debug
    // if (tempCnter>16){
    //    while(1);
    //}
    SET_BIT(elkNodeBitmaps[2],tempCnter);
    return tempCnter;
}

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
                                uint8_t bufIdx)
{
    uint8_t  tempI = 0; 
    uint16_t tempN = 0;         //node-idx which will be removed

    SET_BIT(tempN,startIdx);    // NOTE:-1
    elkNodeBitmaps[bufIdx] = elkNodeBitmaps[bufIdx]-tempN+nodeBitmap;   // NOTE:-2
    svVrfiedBp = svVrfiedBp-tempN+nodeBitmap;                           // x - old + new

    uint16_t tempPrevNodeIdx = elkDualList[bufIdx].stElkList[startIdx].prevNode;    // NOTE:-3
    while(1){
        if(GET_BIT(nodeBitmap,tempI)){
            elkDualList[bufIdx].stElkList[tempI].prevNode = tempPrevNodeIdx;
            if(tempPrevNodeIdx==99){
                elkDualList[bufIdx].nextNode = tempI;
            }else{
                elkDualList[bufIdx].stElkList[tempPrevNodeIdx].nextNode = tempI;
            }
            break;
        }
        tempI++;
    }

    tempN = elkDualList[bufIdx].stElkList[startIdx].nextNode;
    elkDualList[bufIdx].stElkList[tempI].nextNode = tempN;
    if(tempN!=99){
        elkDualList[bufIdx].stElkList[tempN].prevNode = tempI;
    }
}

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
                                uint8_t bufIdx)
{
    uint8_t  tempI = 0; 
    uint16_t tempN = 0;         //node-idx which will be removed

    //NOTE:-1
    SET_BIT(tempN,startIdx);  
    //NOTE:-2
    elkNodeBitmaps[bufIdx] = elkNodeBitmaps[bufIdx]-tempN+nodeBitmap;   // x - old + new
    svVrfiedBp = svVrfiedBp-tempN+nodeBitmap;                           // x - old + new
    //NOTE:-3 find out the number of new nodes.
    uint16_t tempM = nodeBitmap;
    uint8_t  tempCntr = 0;
    for(tempCntr=0; tempM; ++tempCntr){     
        tempM &= (tempM-1);
    }
    //NOTE:-4
    elkDualList[bufIdx].usedNodeNum += (tempCntr-1);
    //NOTE:-5
    tempI = 0;
    uint16_t tempPrevNodeIdx = elkDualList[bufIdx].stElkList[startIdx].prevNode;
    while(tempCntr){
        if(GET_BIT(nodeBitmap,tempI)){
            tempCntr--;
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
    tempM = elkDualList[bufIdx].stElkList[startIdx].nextNode;
    elkDualList[bufIdx].stElkList[tempI-1].nextNode = tempM;
    if(tempM!=99){
        elkDualList[bufIdx].stElkList[tempM].prevNode = tempI-1;
    }
}

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
                                uint8_t bufIdx)
{
    //NOTE:-1
    uint8_t  tempI = startIdx; 
    uint16_t tempN = 0;         //node-idx which will be removed
    while(1){
        SET_BIT(tempN,tempI);
        if(tempI==endIdx){
            break;
        }else{
            tempI = elkDualList[bufIdx].stElkList[tempI].nextNode;
        }
    }
    //NOTE:-2
    elkNodeBitmaps[bufIdx] = elkNodeBitmaps[bufIdx]-tempN+nodeBitmap;   // x - old + new
    svVrfiedBp = svVrfiedBp-tempN+nodeBitmap;                           // x - old + new
    //NOTE:-3 find out the number of new nodes.
    uint16_t tempM = nodeBitmap;
    uint8_t  tempCntr = 0;
    for(tempCntr=0; tempM; ++tempCntr){     
        tempM &= (tempM-1);
    }
    //NOTE:-4
    elkDualList[bufIdx].usedNodeNum += (tempCntr-1);
    //NOTE:-5
    tempI = 0;
    uint16_t tempPrevNodeIdx = elkDualList[bufIdx].stElkList[startIdx].prevNode;
    while(tempCntr){
        if(GET_BIT(nodeBitmap,tempI)){
            tempCntr--;
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
    tempM = elkDualList[bufIdx].stElkList[endIdx].nextNode;
    elkDualList[bufIdx].stElkList[tempI-1].nextNode = tempM;
    if(tempM!=99){
        elkDualList[bufIdx].stElkList[tempM].prevNode = tempI-1;
    }
}

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
                            uint8_t     nodeIdx)
{
    uint8_t  tempNodeIdx;
    uint16_t tempSubCksum;
    uint16_t tempHalf = (addrEnd-addrStart+1)>>1;
    uint8_t bufIdx = elkBufIdx._idx;         //working on working-buffer

    if((addrMiddle-addrStart) > tempHalf){//partial crc
        tempSubCksum = _elk_crc(addrMiddle, addrEnd, (bufIdx^1));
        tempSubCksum = elkListNodes[nodeIdx].subCksum ^ tempSubCksum;   // FIXME:
        tempNodeIdx = findEmptyNode();
        elkListNodes[tempNodeIdx].intvlStart    = addrStart;
        elkListNodes[tempNodeIdx].intvlEnd      = addrMiddle-1;
        elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
        //elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-elkListNodes[tempNodeIdx].intvlEnd-1)>>1;
    }else{//normal crc
        tempNodeIdx = findEmptyNode();
        elkListNodes[tempNodeIdx].intvlStart    = addrStart;
        elkListNodes[tempNodeIdx].intvlEnd      = addrMiddle-1;
        //elkListNodes[tempNodeIdx].paddingNum    = (nvBufSize-elkListNodes[tempNodeIdx].intvlEnd-1)>>1;
        elkListNodes[tempNodeIdx].subCksum      = _elk_crc(addrStart, addrMiddle-1, bufIdx);
    }
}

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
                            uint8_t     nodeIdx)
{
    uint8_t  tempNodeIdx;
    uint16_t tempSubCksum;
    uint16_t tempHalf = (addrEnd-addrStart+1)>>1;
    uint8_t bufIdx = elkBufIdx._idx;         //working on working-buffer

    if((addrEnd-addrMiddle) > tempHalf){//partial crc
        tempSubCksum = _elk_crc(addrStart, addrMiddle, (bufIdx^1));
        tempSubCksum = elkListNodes[nodeIdx].subCksum ^ tempSubCksum;   // FIXME:
        tempNodeIdx = findEmptyNode();
        elkListNodes[tempNodeIdx].intvlStart    = addrMiddle+1;
        elkListNodes[tempNodeIdx].intvlEnd      = addrEnd;
        elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
    }else{//normal crc
        tempNodeIdx = findEmptyNode();
        elkListNodes[tempNodeIdx].intvlStart    = addrMiddle+1;
        elkListNodes[tempNodeIdx].intvlEnd      = addrEnd;
        elkListNodes[tempNodeIdx].subCksum      = _elk_crc(addrMiddle+1, addrEnd, bufIdx);
    }
}


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
                    uint8_t bufIdx)
{
    uint16_t bufBaseStart = (uint16_t)(_threads[0].buffer.buf[bufIdx]);
    uint16_t tempSize = (offsetEnd-offsetStart+1);     //size in byte
    uint16_t tempI;
    uint16_t tempCksum;

    for(tempI=0; tempI<tempSize; tempI=tempI+2){
        HWREG16(CRC_BASE+OFS_CRCDI) = HWREG16(bufBaseStart+offsetStart+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW0) = HWREG16(bufBaseStart+start+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW1) = HWREG16(bufBaseStart+start+tempI+2);
    }
    tempCksum = HWREG16(CRC32_BASE+OFS_CRC32INIRESW0);

    if(offsetStart){
        //tempCksum ^= nvCksumTab[offsetStart];   //FIXME:!!!
        tempCksum ^= 0x0011;
    }

    return tempCksum;
}

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
                        uint16_t tgtIntvlEnd)
{
    uint8_t tempNodeIdx = 0;
    uint16_t tempSubCksum = 0;
    uint16_t tempStart, tempEnd;
    uint8_t workingBufIdx = elkBufIdx._idx;         //working on working-buffer

    elkNodeBitmaps[2]=0;
//|srliu:| go across multiple intervals
    if(svIdxIntvlStart!=svIdxIntvlEnd){
        //---|step1|:seg-upper
        tempStart = elkListNodes[svIdxIntvlStart].intvlStart;       //start offset of start-interval
        tempEnd = elkListNodes[svIdxIntvlStart].intvlEnd;           //end offset of start-interval
        if(tempStart != tgtIntvlStart){
            _elk_dynamic_crc_upper(tempStart, tgtIntvlStart, tempEnd, svIdxIntvlStart);
        }
        //---|step2|:seg-middler
        tempSubCksum = _elk_crc(tgtIntvlStart, tgtIntvlEnd, workingBufIdx);
        tempNodeIdx = findEmptyNode();
        elkListNodes[tempNodeIdx].intvlStart    = tgtIntvlStart;
        elkListNodes[tempNodeIdx].intvlEnd      = tgtIntvlEnd;
        elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
        //---|step3|:seg-lower
        tempStart = elkListNodes[svIdxIntvlEnd].intvlStart;         //start offset of end-interval 
        tempEnd = elkListNodes[svIdxIntvlEnd].intvlEnd;             //end offset of end-interval
        if(tempEnd != tgtIntvlEnd){
            _elk_dynamic_crc_lower(tempStart, tgtIntvlEnd, tempEnd, svIdxIntvlEnd);
        }
        //---|step4|:
        _elk_dlist_replace_mtom(svIdxIntvlStart, svIdxIntvlEnd, elkNodeBitmaps[2], workingBufIdx);
//|srliu| in the same interval
    }else{
        tempStart   = elkListNodes[svIdxIntvlStart].intvlStart;     //start offset of base interval
        tempEnd     = elkListNodes[svIdxIntvlEnd].intvlEnd;         //end offset of base interval
//--|srliu| exactley the same
        if(tempStart==tgtIntvlStart && tempEnd==tgtIntvlEnd){
            tempSubCksum    = _elk_crc(tgtIntvlStart, tgtIntvlEnd, workingBufIdx);
            tempNodeIdx     = findEmptyNode();
            listNodeBackup(svIdxIntvlStart, tempNodeIdx);
            elkListNodes[tempNodeIdx].subCksum = tempSubCksum;
            _elk_dlist_replace_1to1(svIdxIntvlStart, elkNodeBitmaps[2], workingBufIdx);
//--|srliu| interval splitting
        }else{
            //---|step1|:seg-upper
            if(tempStart != tgtIntvlStart){
                _elk_dynamic_crc_upper(tempStart, tgtIntvlStart, tempEnd, svIdxIntvlStart);
            }
            //---|step2|:seg-middle
            tempSubCksum = _elk_crc(tgtIntvlStart, tgtIntvlEnd, workingBufIdx);
            tempNodeIdx = findEmptyNode();
            elkListNodes[tempNodeIdx].intvlStart    = tgtIntvlStart;
            elkListNodes[tempNodeIdx].intvlEnd      = tgtIntvlEnd;
            elkListNodes[tempNodeIdx].subCksum      = tempSubCksum;
            //---|step3|:seg-lower
            if(tempEnd != tgtIntvlEnd){
                _elk_dynamic_crc_lower(tempStart, tgtIntvlEnd, tempEnd, svIdxIntvlEnd);
            }
            //---|step4|:
            _elk_dlist_replace_1tom(svIdxIntvlStart, elkNodeBitmaps[2], workingBufIdx);
        }
    }
}
