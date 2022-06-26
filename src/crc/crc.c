#include <task.h>
#include <apps.h>

#define DATA_LEN 1024
const unsigned int CRC_Init = 0xFFFF;
const unsigned int CRC_Input[] = {
                                    0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                    0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                    0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                    0xc58c, 0xd1e2, 0xe144, 0xb691,
                                    0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                    0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                    0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                    0xc58c, 0xd1e2, 0xe144, 0xb691,
                                    0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                    0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                    0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                    0xc58c, 0xd1e2, 0xe144, 0xb691,
                                    0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                    0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                    0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                    0xc58c, 0xd1e2, 0xe144, 0xb691,
                                    0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                        0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                        0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                        0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                        0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                        0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                        0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                        0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                        0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                        0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                        0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                        0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                        0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                        0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                        0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                        0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                        0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                                                            0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                                                            0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                                                            0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                                                            0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                                                            0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                                                            0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                                                            0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                                                            0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                                                            0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                                                            0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                                                            0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                                                            0x0fc0, 0x1096, 0x5042, 0x0010,           // 16 random 16-bit numbers
                                                                                                            0x7ff7, 0xf86a, 0xb58e, 0x7651,           // these numbers can be
                                                                                                            0x8b88, 0x0679, 0x0123, 0x9599,           // modified if desired
                                                                                                            0xc58c, 0xd1e2, 0xe144, 0xb691,
                                                                                                            0x0fc0, 0x1096, 0x5042, 0x0010
};

/**
 * 2. Shared variable declaration here. (4 bytes)
 */
__shared(
    unsigned int SW_Results;            //-[1]:2
    unsigned int cnt;                   //-[2]:2
)

// Debug defines and flags
#define DEBUG_PORT 3
#define DEBUG_PIN  5

uint8_t full_run_started_crc = 0;   //TODO: __nv

/**
 * 1. TEB declaration here.
 */
TASK(initTask);
TASK(firstByte);
//TASK(secondByte);
TASK(task_finish);

unsigned int CCITT_Update(unsigned int init, unsigned int input);

/**
 * 3. TASK definition here.
 */
TASK(initTask){//-->0, NOTE: R() || W(1,2)
    full_run_started_crc = 1;
    __SET(cnt) = 0;
    __SET(SW_Results) = CRC_Init;

    NEXT(1);
}

TASK(firstByte){//-->1, NOTE: R(1,2) || W(1)
    // First input lower byte
    int i;
    for(i=0;i<100;i++){
    __SET(SW_Results) = CCITT_Update(__GET(SW_Results), CRC_Input[__GET(cnt)] & 0xFF);

    //NEXT(2);
    __SET(SW_Results) = CCITT_Update(__GET(SW_Results), (CRC_Input[__GET(cnt)] >> 8) & 0xFF);
        __SET(cnt)++;
        __delay_cycles(1);
    }
        if (__GET(cnt) < DATA_LEN) {
            NEXT(1);
            //return;
        }
        NEXT(2);
}
/*
TASK(secondByte){//-->2, NOTE: R(1,2) || W(1,2)
    // Then input upper byte

}*/

TASK(task_finish){//-->3, NOTE: R() || W()
    if (full_run_started_crc) {
        full_run_started_crc = 0;
    }
    NEXT(0);
}

// Software algorithm - CCITT CRC16 code
unsigned int CCITT_Update(unsigned int init, unsigned int input)
{
    unsigned int CCITT = (unsigned char) (init >> 8) | (init << 8);
    CCITT ^= input;
    CCITT ^= (unsigned char) (CCITT & 0xFF) >> 4;
    CCITT ^= (CCITT << 8) << 4;
    CCITT ^= ((CCITT & 0xFF) << 4) << 1;
    return CCITT;
}

void _benchmark_crc_init(void){
    __THREAD(0);

    TASK_INIT(0,    initTask,       0,  3);
    TASK_INIT(0,    firstByte,      0,  3);
    TASK_INIT(0,    task_finish,    0,  0);
}
