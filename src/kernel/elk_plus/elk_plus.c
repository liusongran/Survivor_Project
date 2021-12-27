#include "elk_memory_defines.h"
#include "elk_lib.h"
#include "task.h"
#include "profile.h"

extern buffer_idx_t elkBufIdx;          // defined in elk.c

/** 
 * put the access flag to 0 at the begging of each resumption
 * @para.   priority: thread priority
 */
void __elk_plus_init_access(uint8_t priority){
    uint16_t i;
    for(i=0; i<APP_REGION_NUM; i++){
        _threads[priority].protected_mem.wholeMem[i].regAccessed = 0;
    }
}

/** 
 * Init. protected memory bulk
 * @para.   priority: thread priority
 *          wholeStartAddr: in byte, offset in __shared(...)
 *          wholeSize: in byte
 */
void __elk_plus_init_mem(uint8_t priority, uint16_t wholeStartAddr, uint16_t wholeSize){
    uint16_t i;
    uint16_t regionSize = (uint16_t)(wholeSize/APP_REGION_NUM);

    _threads[priority].protected_mem.wholeStartAddr = wholeStartAddr;
    _threads[priority].protected_mem.wholeSize      = wholeSize;
    
    for(i=0; i<APP_REGION_NUM; i++){
        _threads[priority].protected_mem.wholeMem[i].regStartOffset = i*regionSize;
        _threads[priority].protected_mem.wholeMem[i].regEndOffset   = (i+1)*regionSize-1;
        _threads[priority].protected_mem.wholeMem[i].regCksum[0]       = 0;
        _threads[priority].protected_mem.wholeMem[i].regCksum[1]       = 0;
    }
    __elk_plus_init_access(priority);
}

bool __elk_plus_verify(uint16_t idxI, uint16_t idxJ){
    uint16_t i;
    uint16_t tempCksum = 0;
    uint16_t tempStartAddr = (uint16_t)_threads[0].buffer.buf[elkBufIdx.idx]+_threads[0].protected_mem.wholeStartAddr;

    for(i=0; i<APP_REGION_NUM; i++){
        if ((idxI<=_threads[0].protected_mem.wholeMem[i].regEndOffset)&&(idxI>=_threads[0].protected_mem.wholeMem[i].regStartOffset)){
            if(_threads[0].protected_mem.wholeMem[i].regAccessed == 0){
                _threads[0].protected_mem.wholeMem[i].regAccessed = 1;
                tempCksum = __elk_crc ( tempStartAddr+_threads[0].protected_mem.wholeMem[i].regStartOffset,  \
                                        tempStartAddr+_threads[0].protected_mem.wholeMem[i].regEndOffset,    \
                                        elkBufIdx.idx,                                                      \
                                        0);
                if(tempCksum != _threads[0].protected_mem.wholeMem[i].regCksum[elkBufIdx.idx]){
                    return VERIFY_FAILED;
                }
                break;
            }
        }
    }

    for(i=0; i<APP_REGION_NUM; i++){
        if ((idxJ<=_threads[0].protected_mem.wholeMem[i].regEndOffset)&&(idxJ>=_threads[0].protected_mem.wholeMem[i].regStartOffset)){
            if(_threads[0].protected_mem.wholeMem[i].regAccessed == 0){
                _threads[0].protected_mem.wholeMem[i].regAccessed = 1;
                tempCksum = __elk_crc ( tempStartAddr+_threads[0].protected_mem.wholeMem[i].regStartOffset,  \
                                        tempStartAddr+_threads[0].protected_mem.wholeMem[i].regEndOffset,    \
                                        elkBufIdx.idx,                                                      \
                                        0);
                if(tempCksum != _threads[0].protected_mem.wholeMem[i].regCksum[elkBufIdx.idx]){
                    return VERIFY_FAILED;
                }
                break;
            }
        }
    }

    //printk("|%d|%d|%d|%d|\r\n", _threads[0].protected_mem.wholeMem[0].regAccessed, _threads[0].protected_mem.wholeMem[1].regAccessed, _threads[0].protected_mem.wholeMem[2].regAccessed, _threads[0].protected_mem.wholeMem[3].regAccessed);
    //printk("|%d|%d|||\r\n", _threads[0].protected_mem.wholeMem[0].regAccessed, _threads[0].protected_mem.wholeMem[1].regAccessed);
    return VERIFY_PASS;
}

/** 
 * the very first checksum of protected variables for each segment. 
 * NOTE: future version may support dynamic partition here.
 * @para.   bufferIdx: working buffer index
 */
void __elk_plus_first_cksum(uint8_t priority, uint8_t bufferIdx){
    uint8_t i;
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
void __elk_plus_cksum(uint8_t priority, uint8_t bufferIdx){
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
    //printk("|%d|%d|%d|%d|\r\n", _threads[0].protected_mem.wholeMem[0].regAccessed, _threads[0].protected_mem.wholeMem[1].regAccessed, _threads[0].protected_mem.wholeMem[2].regAccessed, _threads[0].protected_mem.wholeMem[3].regAccessed);
}

