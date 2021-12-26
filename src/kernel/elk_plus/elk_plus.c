#include "elk_memory_defines.h"
#include "elk_lib.h"
#include "task.h"

__sv whole_mem_t svWholeMem;
extern buffer_idx_t elkBufIdx;          // defined in elk.c
extern whole_mem_t svWholeMem;          // defined in elk.c

/** 
 * put the access flag to 0 at the begging of each resumption
 * @para.   priority: thread priority
 */
void __elk_plus_init_access(int8_t priority){
    int16_t i;
    for(i=0; i<APP_REGION_NUM; i++){
        _threads[priority].protected_mem.wholeMem[i].regAccessed = 0;
    }
}

/** 
 * Init. protected memory bulk
 * @para.   priority: thread priority
 *          wholeStartAddr: in byte
 *          wholeSize: in byte
 */
void __elk_plus_init_mem(int8_t priority, int16_t wholeStartAddr, int16_t wholeSize){
    int16_t i;
    int16_t regionSize = (int16_t)(wholeSize/APP_REGION_NUM);

    _threads[priority].protected_mem.wholeStartAddr = wholeStartAddr;
    _threads[priority].protected_mem.wholeSize      = wholeSize;
    
    for(i=0; i<APP_REGION_NUM; i++){
        _threads[priority].protected_mem.wholeMem[i].regStartOffset = i*regionSize;
        _threads[priority].protected_mem.wholeMem[i].regEndOffset   = (i+1)*regionSize-1;
        _threads[priority].protected_mem.wholeMem[i].regCksum[0]       = 0;
    }
    __elk_plus_init_access(priority);
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

/** 
 * the very first checksum of protected variables for each segment. 
 * NOTE: future version may support dynamic partition here.
 * @para.   bufferIdx: working buffer index
 */
void __elk_plus_first_cksum(int8_t priority, int8_t bufferIdx){
    int i;
    uint16_t tempCksum  = 0;
    uint16_t tempStartAddr = (uint16_t)_threads[priority].buffer.buf[bufferIdx]+_threads[priority].protected_mem.wholeStartAddr;

    for(i=0; i<APP_REGION_NUM; i++){
        tempCksum = __elk_crc(  tempStartAddr+_threads[priority].protected_mem.wholeMem[i].regStartOffset,  \
                                tempStartAddr+_threads[priority].protected_mem.wholeMem[i].regEndOffset,    \
                                bufferIdx,                                                                  \
                                0);
        _threads[priority].protected_mem.wholeMem[i].regCksum[bufferIdx] = tempCksum;
    }
    __elk_plus_init_access(priority);
}

/** 
 * normal checksum of protected variables for each segment. 
 * NOTE: future version may support dynamic partition here.
 * @para.   bufferIdx: working buffer index
 */
void __elk_plus_cksum(int8_t priority, int8_t bufferIdx){
    int i;
    uint16_t tempCksum = 0;
    uint16_t tempStartAddr = (uint16_t)_threads[priority].buffer.buf[bufferIdx]+_threads[priority].protected_mem.wholeStartAddr;

    for(i=0; i<APP_REGION_NUM; i++){
        if (_threads[priority].protected_mem.wholeMem[i].regAccessed == 1){
            _threads[priority].protected_mem.wholeMem[i].regAccessed = 0;
            tempCksum = __elk_crc(  tempStartAddr+_threads[priority].protected_mem.wholeMem[i].regStartOffset,  \
                                    tempStartAddr+_threads[priority].protected_mem.wholeMem[i].regEndOffset,    \
                                    bufferIdx,                                                                  \
                                    0);
            _threads[priority].protected_mem.wholeMem[i].regCksum[bufferIdx] = tempCksum;
        }
    }
}

