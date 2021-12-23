#include "main.h"
#include "apps.h"
#include "elk.h"
#include "simulation.h"
#include "profile.h"

extern int64_t  _chg_curBgt;
extern uint16_t initStart;      
extern uint16_t initEnd;
extern uint64_t initSum;

extern int16_t calbriBgt;

extern uint32_t delta;
extern uint64_t total;

extern uint8_t svIntervalNum;
extern uint16_t svVrfiedBp;
extern uint16_t svMarkedBp;
extern uint8_t elkCurTaskID;
extern uint8_t testFlg;
__nv uint16_t _chg_num=0;

extern uint16_t roundNum;

extern int32_t checkPeriod;
extern uint8_t fail_flag;
extern uint32_t nvFailedNum;



static void __chg_bgt_get(){
    _chg_curBgt = SIMU_ON_PERIOD;
    _chg_num++;
#ifdef TOTALRECALL
    checkPeriod = CKSUM_FRQ;
#endif
}

extern uint64_t taskSum;
extern uint16_t nvTaskNum;

int main(void){
    __mcu_init();
    __gpio_init();
    __uart_init();
    __timerA_init();

    while(1){
        __chg_bgt_get();

        if(nvInited){
#ifdef TOTALRECALL
//PRB_START(init)
           // __total_verify_nv();
//PRB_END(init)
            //delta = 2500*((uint32_t)(CFG_SSIZE>>3));
            delta = 0;
if(_chg_curBgt<delta){
    total += _chg_curBgt;
    _chg_curBgt = 0;
}else{
    total += delta;
    _chg_curBgt -= delta;
    checkPeriod -= delta;
}
            if(fail_flag){
                nvFailedNum++;
                fail_flag = 0;
                nvInited=0;
                testFlg=0;
            }
#endif

#ifdef ELK
PRB_START(init)
            if(VERIFY_PASS==__elk_check_nv()){
                elkCurMode = 0;
                svIntervalNum = 0;
                svVrfiedBp = 0;
                svMarkedBp = 0;
                __elk_backup(0, elkCurTaskID);
            }else{
                nvInited=0;
                testFlg=0;
            }
PRB_END(init)
if(_chg_curBgt<delta){
    total += _chg_curBgt;
    _chg_curBgt = 0;
    break;
}else{
    total += delta;
    _chg_curBgt -= delta;
}
#endif
        }

        while(1){
            if(_chg_curBgt<=0){
                break;
            }
//PRB_START(init)

#ifdef ELK
            if(!nvInited&&!testFlg){
                __elk_init();
                ENTER_CRITICAL
            }
#endif
            if(!nvInited||testFlg){
                testFlg = 0;
#ifdef TOTALRECALL
                elkCurTaskID = 0;
                taskSum = 0;
                nvTaskNum = 0;
#endif
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

            __scheduler_run();          //kick-off run-time system.
        }
    }
    printk("[ERROR] Should not reach here!\r\n");
    return 0;
}
