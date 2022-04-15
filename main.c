#include "main.h"
#include "mcu.h"
#include "elk_api.h"
#include "apps.h"

/* 
 * ---------------
 * Energy charger.
 * ---------------
 */
extern int64_t _chg_curBgt;
extern uint16_t _chg_num;
static void __chg_bgt_get_elk(){
    _chg_curBgt = ON_PERIOD;
    _chg_num++;
}
extern int32_t checkPeriod;
extern uint32_t nvFailedNum;
extern uint16_t roundNum;
static void __chg_bgt_get_total(){
    _chg_curBgt = ON_PERIOD;
    while(_chg_num>FAIL_NUM){
        //printk("--||Total APP num:%d.\r\n",roundNum);
        //printk("--||Failed APP num:%lu.\r\n",nvFailedNum);
        while(1);
    }
    _chg_num++;
    checkPeriod = CKSUM_FRQ;
}

/* 
 * --------------
 * Entry of Main.
 * --------------
 */
extern uint8_t svIntervalNum;   //??
extern uint16_t nvInited;
extern uint8_t testFlg;
extern uint64_t taskSum;
extern uint16_t nvTaskNum;
extern uint8_t elkCurTaskID;
extern uint16_t svVrfiedBp; extern uint16_t svMarkedBp;
extern uint32_t delta;      extern uint64_t total;
extern uint16_t initStart;  extern uint16_t initEnd;    extern uint64_t initSum;
extern uint16_t backupStart;extern uint16_t backupEnd;  extern uint64_t backupSum;
int main(void)
{
    __mcu_init();
    UART_initGPIO();
    UART_init();
    __timerA_init();

    printf("teststssssttt.\r\n");

    while(1){
        __chg_bgt_get_elk();
        if(nvInited){
PRB_START(init)
            if(VERIFY_PASS==__elk_check_nv()){
PRB_END(init)
                svIntervalNum = 0;
                svVrfiedBp = 0;
                svMarkedBp = 0;
PRB_START(backup)
                __elk_backup(elkCurTaskID);
PRB_START(backup)
            }else{
                nvInited=0;
                testFlg=0;
            }
        }
        while(1){
            if(_chg_curBgt<=0){
                break;
            }
            if((!nvInited) && (!testFlg)){ //-not first boot AND finish one round, only used in profile
                __elk_init();
            }
            if((!nvInited) || (testFlg)){
                testFlg = 0;
#ifdef SRT
                _benchmark_sort_init();
#endif
#ifdef AR
                _benchmark_ar_init();
#endif
#ifdef DIJ
                _benchmark_dijkstra_init();
#endif
#ifdef BC
                _benchmark_bc_init();
#endif
#ifdef CK
                _benchmark_cuckoo_init();
#endif
#ifdef RSA
                _benchmark_rsa_init();
#endif
#ifdef CRC
                _benchmark_crc_init();
#endif
#ifdef CEM
                _benchmark_cem_init();
#endif
#ifdef FFT
                _benchmark_fft_init();
#endif
#ifdef ADPCM
                _benchmark_adpcm_init();
#endif
            }
            __scheduler_run();  //kick-off run-time system.
        }
    }
    printk("[ERROR] Should not reach here!\r\n");
    return 0;
}
