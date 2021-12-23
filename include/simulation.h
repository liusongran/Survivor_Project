/** Simulation related macros defined here. */
#include "profile.h"

#define SIMU_FAIL_FREQ              2000000000
#define SIMU_FAIL_FRQ               (SIMU_FAIL_FREQ)            //in us
#define SIMU_CKSUM_FRQ              (SIMU_FAIL_FRQ>>1)          //For total recall

#define SIMU_TEST_PERIOD            200                         //in seconds
#define SIMU_FAIL_NUM               (SIMU_TEST_PERIOD*1000000/SIMU_FAIL_FRQ)

#define SIMU_ON_PERIOD              (SIMU_FAIL_FRQ)             //us
#define SIMU_WARNING                (SIMU_FAIL_FRQ-1000)        //us

#ifdef DEBUG_ENABLE
#define SIMU_ITERATION              3
#else
#define SIMU_ITERATION              50
#endif
