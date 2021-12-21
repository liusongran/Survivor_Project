#include "elk_lib.h"

/* --------------
 * [__elk_crc]: done!!
 * LOG: memory-range crc.
 * 1. find the real address;
 * 2. compute the corresponding cksum;
 * 3. update the final cksum by padding zeros;
 * -----------
 */

uint16_t __elk_crc(uint16_t start, uint16_t end, uint8_t bufIdx, uint16_t paddings) {
    //uint16_t bufAddrStart = (uint16_t)(_threads[0].buffer.buf[bufIdx]);
    uint16_t tempSize = (end-start+1);     //size in word
    uint16_t tempI;

    //crcSizeSum += tempSize>>1;

    uint16_t tempCksum;
    //HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
//#if TRACE_CRC_TIME
//PRB_START(crcPrf)
//#endif
    for(tempI=0;tempI<tempSize;tempI=tempI+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(start+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW0) = HWREG16(bufAddrStart+start+tempI);
        //HWREG16(CRC32_BASE + OFS_CRC32DIW1) = HWREG16(bufAddrStart+start+tempI+2);
//#if TRACE_CRC
//        crcSizeSum += 2;
//#endif
    }
    tempCksum = HWREG16(CRC32_BASE + OFS_CRC32INIRESW0);
//#if TRACE_CRC_TIME
//PRB_END(crcPrf)
//#endif
    if(paddings){
        //tempCksum ^= nvCksumTab[paddings-1];
        tempCksum ^= 0x0011;
    }/*
    while(paddings){
        HWREG16(CRC_BASE + OFS_CRCDI) = 0x0000;
        paddings--;
    }*/
    return tempCksum;
}
