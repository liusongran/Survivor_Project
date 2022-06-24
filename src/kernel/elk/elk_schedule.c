#include "elk_api.h"

/**
 * ELK system global variables
 * - __nv: Const variables on FRAM, which will no change.
 * - __elk:
 * - __elk_du:
 * - __sv: Volatile variables on SRAM, no need to CRC and BACKUP to FRAM.
 */
__nv thread_t _threads[MAX_THREAD_NUM];
__nv buffer_idx_t elkBufIdx;                        //this to nv
__nv buffer_idx_t elkDualIdx;                        //this to nv

__nv uint8_t elkCurTaskID = 0;                      //this in elk
__elk_du list_node_t elkListNodes[MAX_SUB_CKSUM_NUM];
__elk_du elk_list_t elkDualList[2];
__nv uint16_t elkNodeBitmaps[3];                    //0:backup, 1:working, 2:this-round

__sv uint16_t svVrfiedBp = 0;                       // verified bitmap
__sv uint8_t  svIntervalNum = 0;                    // temp valid interval number
__sv ck_set_t svIntvlArray[MAX_TASK_NUM] = {0};     // temp interval array
__sv uint16_t svMergeBp = 0;                        // merged bitmap
__sv uint16_t svMarkedBp = 0;                       // marked node bitmap
__sv uint8_t  svIdxIntvlStart = 0;                  // target start interval index.
__sv uint8_t  svIdxIntvlEnd = 0;                    // target end interval index.

__nv uint16_t nvInited = 0;
__nv uint32_t crcSeed = 0xBEEF;
__nv uint16_t nvListSize = sizeof(elk_list_t);
__nv uint16_t nvBufSize=0;
__nv uint16_t nvCksumTab[MAX_CKSUM_TAB_NUM];        // padding dict.
/**
 * Simulator used global variables
 */
__nv uint8_t testFlg = 0;           //-flag to show if we need to reset ELK global variables
__nv uint16_t roundNum = 0;         //-Benchmark round

/**
 * Profile used global variables
 */
__nv uint8_t fail_flag = 0;     //??
__nv int16_t calbriBgt=0;       //??
__nv int32_t checkPeriod = 0;   //??

__nv uint32_t delta = 0;
__nv uint32_t crcSizeSum = 0;
__nv uint64_t total = 0;
__nv int64_t _chg_curBgt = 0;
__nv uint16_t nvTaskNum = 0;
__nv uint32_t nvFailedNum = 0;
__nv uint16_t _chg_num=0;

__nv uint16_t verifyStart = 0;  __nv uint16_t verifyEnd = 0;    __nv uint64_t verifySum = 0;
__nv uint16_t backupStart = 0;  __nv uint16_t backupEnd = 0;    __nv uint64_t backupSum = 0;
__nv uint16_t taskStart = 0;    __nv uint16_t taskEnd = 0;      __nv uint64_t taskSum = 0;
__nv uint16_t cksumStart = 0;   __nv uint16_t cksumEnd = 0;     __nv uint64_t cksumSum = 0;
__nv uint16_t updateStart = 0;  __nv uint16_t updateEnd = 0;    __nv uint64_t updateSum = 0;

__nv uint16_t crcPrfStart = 0;  __nv uint16_t crcPrfEnd = 0;    __nv uint64_t crcPrfSum = 0;
__nv uint16_t initStart = 0;    __nv uint16_t initEnd = 0;      __nv uint64_t initSum = 0;
__nv uint16_t markStart = 0;    __nv uint16_t markEnd = 0;      __nv uint64_t markSum = 0;
/**
 * TODO:...
 */
__elk uint8_t elkCurMode = 0;                       //0->normal, 1->intermittent
__elk uint8_t elkClearMark = 0;                     //0->normal, 1-start to clear
__elk uint16_t elkTotalCksum;
__nv uint16_t tempRndCntr=0;
__nv uint8_t bgtFlag = 0;

/**
 * TotalRECALL used variables.
 */
__nv uint16_t nvTotalCksum;


/* ------------------
 * [__scheduler_run]: done!!
 * LOG: scheduler of ELK.
 */
void __scheduler_run()
{
    uint8_t tempResult;
    volatile uint8_t tempTaskID;
    cksum_temp_t tempCksum;

    while(1){
        if(nvInited){  //branch-1 ---> system is already booted.
        printk("---------------------------------------\r\n");
//NOTE: Step1 - verify
PRB_START(verify)
            if(_threads[0].task_array[elkCurTaskID].ck_set.end_used_offset != 0){
                tempResult = __elk_verify(elkCurTaskID);
            }
PRB_END(verify)
            if(tempResult==VERIFY_FAILED){
                nvInited = 0;
                break;
            }
        }
//NOTE: Step2 - backup
PRB_START(backup)
        __elk_backup(elkCurTaskID);
PRB_END(backup)

#if TRACE_TASK==1
            printk("|!!!3.|running|task%d: ",elkCurTaskID);
#endif
//NOTE: Step3 - task
PRB_START(task)
            tempTaskID = (uint8_t)((taskfun_t)(_threads[0].task_array[elkCurTaskID].fun_entry))(_threads[0].buffer.buf[1]);
PRB_END(task)
            nvTaskNum++;
#if TRACE_TASK==1
            printk("%lu. total:%u.\r\n",delta,nvTaskNum);
#endif
#if (DEBUG_TASK==1)
            printf("|!!!3.|running|intvStart:%d, intvEnd:%d.\r\n", _threads[0].task_array[elkCurTaskID].ck_set.start_used_offset, _threads[0].task_array[elkCurTaskID].ck_set.end_used_offset);
#endif
//NOTE: Step4 - checksum
PRB_START(cksum)
            if(nvInited){
                __elk_checksum(elkCurTaskID);                 
            }else{
                __elk_first_cksum();
                nvInited = 1;
            }
PRB_END(cksum)

//NOTE: Step5 - commit
PRB_START(update)
            tempCksum = __elk_update_nv();
PRB_END(update)

//NOTE: Step6 - DO NOT COUNT
            __elk_commit(tempTaskID, tempCksum);

        if(elkCurTaskID==0){
            P1OUT = 0b100011;   //set P1.5, clear P1.4
            testFlg = 1;
            roundNum++;
            if(roundNum<ITER){
                printk("|APP num:%d.\r\n",roundNum);
#if TRACE_CRC_TIME==1
                printk("|CRCprf:%u(100us)\r\n",     (uint32_t)(crcPrfSum)/100);
#endif
#if TRACE_CRC_SIZE==1
                printk("|Total size: %lu(bytes).\r\n", (uint32_t) crcSizeSum);
#endif
                printk("|InitSum:%lu(100us)\r\n",   (uint32_t)(initSum)/100);
                printk("|BackupSum:%lu(100us)\r\n", (uint32_t)(backupSum)/100);
                printk("|CksumSum:%lu(100us)\r\n",  (uint32_t)(cksumSum)/100);
                printk("|UpdateSum:%lu(100us)\r\n", (uint32_t)(updateSum)/100);
                printk("|TaskSum:%lu(100us), ",     (uint32_t)taskSum/100); printk("num:%d.\r\n",nvTaskNum);
                printk("|VerifySum:%lu(100us)\r\n", (uint32_t)(verifySum)/100);
                printk("|Total:%lu(100us)\r\n",     (uint32_t)(total)/100);
                while(1);
            }

            __delay_cycles(100);
            P1OUT = 0b010000;   //set P1.4, clear P1.5
            break;
        }
    }
}


/* -------------
 * [__total_cksum_nv]: done!!
 * LOG: ELK variables inited.
 */
void __total_cksum_nv(){
    int i,j;
    HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    for(j=0;j<SSIZE;j++){
    for(i=0;i<(SRAM_SIZE+128);i=i+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(SRAM_START+i);
    }
    }
    //crcSizeSum += SRAM_SIZE*SSIZE;
    nvTotalCksum = HWREG16(CRC_BASE + OFS_CRCINIRES);
}

uint16_t __total_verify_nv(){
    int i,j;
    uint16_t tempCksum;
    HWREG16(CRC_BASE + OFS_CRCINIRES) = crcSeed;
    for(j=0;j<SSIZE;j++){
    for(i=0;i<(SRAM_SIZE+128);i=i+2){
        HWREG16(CRC_BASE + OFS_CRCDI) = HWREG16(SRAM_START+i);
    }
    }
    //crcSizeSum += SRAM_SIZE*SSIZE;
    tempCksum = HWREG16(CRC_BASE + OFS_CRCINIRES);
    if(tempCksum==nvTotalCksum){
        return VERIFY_PASS;
    }else{
        return VERIFY_FAILED;
    }
}
