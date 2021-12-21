#include "elk_memory_defines.h"
#include "elk_lib.h"
#include "task.h"

__sv whole_mem_t svWholeMem;
extern buffer_idx_t elkBufIdx;          // defined in elk.c
extern whole_mem_t svWholeMem;          // defined in elk.c

/** put the access flag to 0 at the begging of each resumption */
void __elk_plus_init_access(){
    int i;
    for(i=0; i<APP_REGION_NUM; i++){
        svWholeMem.wholeMem[i].regAccessed = 0;
    }
}

void __elk_plus_init_mem(uint16_t wholeStartAddr, uint16_t wholeSize){
    svWholeMem.wholeStartAddr   = wholeStartAddr;
    svWholeMem.wholeSize        = wholeSize;
    uint16_t i;
    uint16_t regionSize = (uint16_t)(wholeSize/APP_REGION_NUM);
    for(i=0; i<APP_REGION_NUM; i++){
        svWholeMem.wholeMem[i].regStartOffset = i*regionSize;
        svWholeMem.wholeMem[i].regEndOffset = (i+1)*regionSize-1;
        svWholeMem.wholeMem[i].regCksum = 0;
    }
    __elk_plus_init_access();
}

bool __elk_plus_verify(uint16_t idxI, uint16_t idxJ){
    uint16_t i;
    uint16_t tempCksum = 0;

    for(i=0; i<APP_REGION_NUM; i++){
        if ((idxI<=svWholeMem.wholeMem[i].regEndOffset)&&(idxI>=svWholeMem.wholeMem[i].regStartOffset)){
            if(svWholeMem.wholeMem[i].regAccessed == 0){
                svWholeMem.wholeMem[i].regAccessed = 1;
                tempCksum = __elk_crc ( svWholeMem.wholeStartAddr+2*svWholeMem.wholeMem[i].regStartOffset,  \
                                        svWholeMem.wholeStartAddr+2*svWholeMem.wholeMem[i].regEndOffset,    \
                                        elkBufIdx.idx,                                                      \
                                        0);
                if(tempCksum != svWholeMem.wholeMem[i].regCksum){
                    return VERIFY_FAILED;
                }
            }
        }
    }
    return VERIFY_PASS;
}

void __elk_plus_cksum(){
    int i;
    uint16_t tempCksum = 0;

    for(i=0; i<APP_REGION_NUM; i++){
        if (svWholeMem.wholeMem[i].regAccessed == 1){
            tempCksum = __elk_crc ( svWholeMem.wholeStartAddr+2*svWholeMem.wholeMem[i].regStartOffset,  \
                                    svWholeMem.wholeStartAddr+2*svWholeMem.wholeMem[i].regEndOffset,    \
                                    elkBufIdx._idx,                                                      \
                                    0);
            svWholeMem.wholeMem[i].regCksum = tempCksum;
        }
    }
}

