#include <task.h>
//#include <mspbase.h>
#include <./src/apps/Lib/dsplib/include/DSPLib.h>
#include <apps.h>

#define N_SAMPLES     128
#define STAGE1_STEP   (2)
#define STAGE2_STEP   (STAGE1_STEP*2)
#define STAGE3_STEP   (STAGE2_STEP*2)



#define MSP_OVERFLOW_MAX        (INT16_MAX >> 2)
#define MSP_OVERFLOW_MIN        (INT16_MIN >> 2)

__nv msp_cmplx_twiddle_table_128_q15[DSPLIB_TABLE_OFFSET+128] = {
    0x0080, 0x0000,
    0x7FFF, 0x0000, 0x7FD9, 0xF9B8, 0x7F62, 0xF374, 0x7E9D, 0xED38,
    0x7D8A, 0xE707, 0x7C2A, 0xE0E6, 0x7A7D, 0xDAD8, 0x7885, 0xD4E1,
    0x7642, 0xCF04, 0x73B6, 0xC946, 0x70E3, 0xC3A9, 0x6DCA, 0xBE32,
    0x6A6E, 0xB8E3, 0x66D0, 0xB3C0, 0x62F2, 0xAECC, 0x5ED7, 0xAA0A,
    0x5A82, 0xA57E, 0x55F6, 0xA129, 0x5134, 0x9D0E, 0x4C40, 0x9930,
    0x471D, 0x9592, 0x41CE, 0x9236, 0x3C57, 0x8F1D, 0x36BA, 0x8C4A,
    0x30FC, 0x89BE, 0x2B1F, 0x877B, 0x2528, 0x8583, 0x1F1A, 0x83D6,
    0x18F9, 0x8276, 0x12C8, 0x8163, 0x0C8C, 0x809E, 0x0648, 0x8027,
    0x0000, 0x8001, 0xF9B8, 0x8027, 0xF374, 0x809E, 0xED38, 0x8163,
    0xE707, 0x8276, 0xE0E6, 0x83D6, 0xDAD8, 0x8583, 0xD4E1, 0x877B,
    0xCF04, 0x89BE, 0xC946, 0x8C4A, 0xC3A9, 0x8F1D, 0xBE32, 0x9236,
    0xB8E3, 0x9592, 0xB3C0, 0x9930, 0xAECC, 0x9D0E, 0xAA0A, 0xA129,
    0xA57E, 0xA57E, 0xA129, 0xAA0A, 0x9D0E, 0xAECC, 0x9930, 0xB3C0,
    0x9592, 0xB8E3, 0x9236, 0xBE32, 0x8F1D, 0xC3A9, 0x8C4A, 0xC946,
    0x89BE, 0xCF04, 0x877B, 0xD4E1, 0x8583, 0xDAD8, 0x83D6, 0xE0E6,
    0x8276, 0xE707, 0x8163, 0xED38, 0x809E, 0xF374, 0x8027, 0xF9B8
};

const _q15 a11[N_SAMPLES] = {
     -9634,  32767,  19555,  10950,   4736,   9785,  -7267, -11995,
     -3027,  16331,  -6896, -14342,  -5820,  -7920, -18265, -13059,
      8401,  25561,   8439,   1384,  18253,   6700, -12424,  -5112,
     13543,  11971,  -3546,  -9726,  -5575, -17249, -27358,  -2533,
     12940,  21431,   -195,  13989,  17916,   -917, -14959, -11988,
     16960,  -5800, -23360, -10399, -11280, -18164, -18909,   7518,
     21235,  18131,   6905,  16367,  13048,  -4587, -13190,   6038,
     17784, -13350, -16788, -13129, -12414, -21115, -10926,  10118,
     21871,  11366,   6463,  15922,   5996, -17453,  -7421,   3532,
     13302, -13445, -17979,  -7560, -22309, -24273,  -6252,  31085,
     24774,  -3417,  12622,   7503,  -4640, -13781,   4571,  13492,
      7644, -16293,  -8240,  -5284, -21173, -16059,   8996,  21233,
      6603,  14525,  11112,   9318,  -8076, -10524,   5138,  10194,
     -7234,  -8000, -10695,  -9121, -23149, -12176,  12841,  21670,
      8944,   2334,  15443,  -1785, -14722,  -2664,   9938,   4850,
    -10809, -19666,  -2266,  -7774, -19165,  -1347,  25085,  13696
};
const _q15 a21[N_SAMPLES] = {
      2368,  32090,   8756,  10699,  13623,   6942,  -8188, -12738,
     19991,  26983,  -7724,  -1734,  -5687,  -9895, -20479, -18817,
     10994,  31821,  19079,  13492,  17731,   1600,  -9427,  -2111,
     13519,  15376,  -5966, -15473,  -5518, -18238, -21466, -12918,
     15753,  18670,  -4585,  16285,  16136,  -4276,  -9099,  -8966,
     14502,   2604, -12500,  -9712, -11279, -20699, -23232,   6749,
     29785,  21627,   2882,  13602,   5764, -13595, -13068,  13633,
     10535,  -2795, -16105,  -3039, -15930, -24515, -12485,  19765,
     31459,  11511,   1390,  11842,   -246,  -3573,  -9595,  16332,
      7843,  -7198, -16630, -12863, -23007, -22821,  -4767,  21642,
     26817,   8753,  11960,  20897,  -7211, -11885,   5105,  14060,
      1900, -19839, -15006,  -7921, -19055,  -9074,   4928,  26203,
     14406,  -1702,  12203,   1262,  -7118, -15278,   9731,  10187,
     -5582, -17418,  -5093,  -9617, -17722, -12397,   8496,  19928,
     15583,   4159,  19332,   2324,  -8459, -12868,  13375,    958,
       105,  -8586,   -829, -23366, -26078,  -1201,  28851,  17480
};
const _q15 a31[N_SAMPLES] = {
      -934,  25596,  19798,   9900,  19442,  17539, -11568, -23760,
      7954,   6497,  -1440, -14490,    899,  -9621, -25076, -16363,
     12240,  24792,   9883,   1809,  15676,    973, -16612,  -3324,
      9231,   9891,  -5046, -14989,  -2210, -15795, -20283,  -4940,
     18929,  17863,   3673,   8494,  12386,  -2668, -10474,  -2656,
     19382,   1285,  -9330, -12983,  -6489, -15796, -23590,   1992,
     32767,  20294,   4638,  15629,   9236,  -5413,  -6595,   -178,
     17895,   1930, -14450,   -261, -10131, -29193, -23833,  13041,
     26956,  12150,   9484,  12187,   4910, -20500,  -9921,   9144,
     10645, -10835, -17794,  -6525, -19180, -27059,   -174,  22306,
     21642,  11017,  11522,  15835,   -640,  -9500,   3416,  20704,
      1018, -14462,  -2383, -10305, -21573, -15911,   8266,  30606,
     18737,   7850,  10385,  11308, -16395,  -9977,  12460,  13113,
     -2952,  -5618,  -3085, -11343, -21754,  -9374,  11755,  21130,
     14555,   8425,  17128,  -2574, -14061,  -1540,   7999,   7571,
    -13093, -15861,  -6853, -26235, -32372,  -5027,  19054,  13127
};
#define TWIDDLE_TABLE msp_cmplx_twiddle_table_128_q15

/*
 * 1. TEB declaration here.
 */
TASK(task_init);            //-(0)
TASK(task_sample);      //-(1)
TASK(task_fft_prologue);          //-(3)
TASK(task_fft_stage_1);          //-(3)
TASK(task_fft_stage_2);          //-(3)
TASK(task_fft_stage_3);          //-(3)
TASK(task_fft_epilogue);          //-(3)
TASK(task_done);

static inline void msp_cmplx_overflow_q15(int16_t *src, bool *overflow);
static inline void msp_cmplx_btfly_auto_q15(int16_t *srcA, int16_t *srcB, const _q15 *coeff, bool *scale, bool *overflow);
static inline void msp_cmplx_btfly_c0_auto_q15(int16_t *srcA, int16_t *srcB, bool *scale, bool *overflow);
static inline void msp_cmplx_btfly_c1_auto_q15(int16_t *srcA, int16_t *srcB, bool *scale, bool *overflow);

DSPLIB_DATA(fft_array_copy, MSP_ALIGN_FFT_Q15(N_SAMPLES))
_q15 fft_array_copy[N_SAMPLES];


/*
 * 2. Shared variable declaration here. (206 bytes)
 */
//__shared(
//uint32_t iteration;
//uint16_t fft_shift;
//uint16_t fft_scale;
//uint16_t fft_overflow;
//_q15 fft_array[N_SAMPLES];
//)

__shared(
    uint32_t iteration;         //-[1]:1 * 4
    uint16_t fft_shift;         //-[2]:1 * 2
    uint16_t fft_scale;         //-[3]:1 * 2
    uint16_t fft_overflow;      //-[4]:1 * 2
    _q15 fft_array[N_SAMPLES];  //-[5]:N_SAMPLES * 2 = 256
)

//TASK(task_init) {//-->0, NOTE: [R-or-RW() || RW-or-W-or-WR(1,2)]
//TASK(task_fft_prologue){//-->1, NOTE: [R-or-RW() || RW-or-W-or-WR(1,2,3,4,5)]
//TASK(task_fft_stage_2){//-->2, NOTE: [R-or-RW(3,4,5) || RW-or-W-or-WR(2,3,4,5)]
//TASK(task_fft_epilogue){//-->4, NOTE: [R-or-RW(5) || RW-or-W-or-WR(5)]
//TASK(task_done){//-->4, NOTE: [R-or-RW(1) || RW-or-W-or-WR()]

static _q15 get_sample(const _q15* ptr, uint16_t sample_idx)
{
    __delay_cycles(20);
    return ptr[sample_idx];
}

TASK(task_init){//0
    __SET(iteration)=0;
    NEXT(1);
}


TASK(task_fft_prologue){//1
    uint16_t i, length;

    uint16_t sample_idx;

    __SET(iteration)++;
    const _q15* ptr;
    switch (__GET(iteration) % 3) {
        case 1:
            ptr = a11;
            break;
        case 2:
            ptr = a21;
            break;
        case 0:
            ptr = a31;
            break;
        default:
            ptr = a11;
    }

    for (sample_idx = 0; sample_idx < N_SAMPLES; sample_idx++) {
        __SET(fft_array[sample_idx]) = get_sample(ptr, sample_idx);
    }



    // Pull array.
    for (i = 0; i < N_SAMPLES; i++) {
        fft_array_copy[i] = __GET(fft_array[i]);
    }

    length = N_SAMPLES / 2;

    // Bit reverse the order of the inputs.
    msp_cmplx_bitrev_q15_params paramsBitRev;
    paramsBitRev.length = length;

    // Perform bit reversal on source data.
    msp_cmplx_bitrev_q15(&paramsBitRev, fft_array_copy);

    // Push array.
    for (i = 0; i < N_SAMPLES; i++) {
        __SET(fft_array[i]) = fft_array_copy[i];
    }

        bool scale;                         // scale flag
        bool overflow;                      // overflow flag
        uint16_t j;                      // loop counters
        int16_t *srcPtr;                    // local source pointer

        length = N_SAMPLES / 2;

        // Pull array.
        for (i = 0; i < N_SAMPLES; i++) {
            fft_array_copy[i] = __GET(fft_array[i]);
        }

        // Check for initial overflow.
        overflow = false;
        for (i = 0 ; i < length; i++) {
            msp_cmplx_overflow_q15(&fft_array_copy[i*2], &overflow);
        }

        // Stage 1.
        scale = overflow;
        overflow = false;

        // Initialize shift result.
        __SET(fft_shift) = scale ? 1 : 0;

        if (STAGE1_STEP <= length) {
            for (j = 0; j < length; j += STAGE1_STEP) {
                srcPtr = fft_array_copy + j*2;
                msp_cmplx_btfly_c0_auto_q15(&srcPtr[0], &srcPtr[0+STAGE1_STEP], &scale, &overflow);
            }
        }

        // Push flags.
        __SET(fft_scale) = scale;
        __SET(fft_overflow) = overflow;

        // Push array.
        for (i = 0; i < N_SAMPLES; i++) {
            __SET(fft_array[i]) = fft_array_copy[i];
        }

        //NEXT(4);
    NEXT(2);
}


TASK(task_fft_stage_2){
    uint16_t length;
    bool scale;                         // scale flag
    bool overflow;                      // overflow flag
    uint16_t i, j;                      // loop counters
    int16_t *srcPtr;                    // local source pointer

    length = N_SAMPLES / 2;

    // Pull flags.
    scale = __GET(fft_scale);
    overflow = __GET(fft_overflow);

    // Pull array.
    for (i = 0; i < N_SAMPLES; i++) {
        fft_array_copy[i] = __GET(fft_array[i]);
    }

    // Stage 2.
    scale = overflow;
    overflow = false;
    __SET(fft_shift) += scale ? 1 : 0;
    if (STAGE2_STEP <= length) {
        for (j = 0; j < length; j += STAGE2_STEP) {
            srcPtr = fft_array_copy + j*2;
            msp_cmplx_btfly_c0_auto_q15(&srcPtr[0], &srcPtr[0+STAGE2_STEP], &scale, &overflow);
            msp_cmplx_btfly_c1_auto_q15(&srcPtr[2], &srcPtr[2+STAGE2_STEP], &scale, &overflow);
        }
    }

    // Push flags.
    __SET(fft_scale) = scale;
    __SET(fft_overflow) = overflow;

    // Push array.
    for (i = 0; i < N_SAMPLES; i++) {
        __SET(fft_array[i]) = fft_array_copy[i];
    }

    //NEXT(5);
        uint16_t step;                      // step size
        uint16_t twiddleIndex;              // twiddle table index
        uint16_t twiddleIncrement;          // twiddle table increment
        const _q15 *twiddlePtr;             // twiddle table pointer

        length = N_SAMPLES / 2;

        // Pull flags.
        scale = __GET(fft_scale);
        overflow = __GET(fft_overflow);

        // Pull array.
        for (i = 0; i < N_SAMPLES; i++) {
            fft_array_copy[i] = __GET(fft_array[i]);
        }

        // Initialize step size, twiddle angle increment and twiddle table pointer.
        step = STAGE3_STEP;
        twiddleIncrement = 2*(*(uint16_t*)TWIDDLE_TABLE)/STAGE3_STEP;
        //twiddlePtr = &TWIDDLE_TABLE[DSPLIB_TABLE_OFFSET];

        // If MPY32 is available save control context and set to fractional mode.
    #if defined(__MSP430_HAS_MPY32__)
        uint16_t ui16MPYState = MPY32CTL0;
        MPY32CTL0 = MPYFRAC | MPYDLYWRTEN;
    #endif

        // Stage 3 -> log2(step).
        while (step <= length) {
            // Reset the twiddle angle index.
            twiddleIndex = 0;

            // Set scale and overflow flags.
            scale = overflow;
            overflow = false;
            __SET(fft_shift) += scale ? 1 : 0;

            for (i = 0; i < (step/2); i++) {
                // Perform butterfly operations on complex pairs.
                for (j = i; j < length; j += step) {
                    srcPtr = fft_array_copy + j*2;
                    msp_cmplx_btfly_auto_q15(srcPtr, srcPtr + step, &twiddlePtr[twiddleIndex], &scale, &overflow);
                }

                // Increment twiddle table index.
                twiddleIndex += twiddleIncrement;
            }
            // Double the step size and halve the increment factor.
            step *= 2;
            twiddleIncrement = twiddleIncrement/2;
        }

        // Restore MPY32 control context.
    #if defined(__MSP430_HAS_MPY32__)
        MPY32CTL0 = ui16MPYState;
    #endif

        // Push array.
        for (i = 0; i < N_SAMPLES; i++) {
            __SET(fft_array[i]) = fft_array_copy[i];
        }

        NEXT(3);
}

TASK(task_fft_epilogue){
    uint16_t i;

    // Pull array.
    for (i = 0; i < N_SAMPLES; i++) {
        fft_array_copy[i] = __GET(fft_array[i]);
    }

    // Initialize split operation params structure.
    msp_split_q15_params paramsSplit;
    paramsSplit.length = N_SAMPLES;
    paramsSplit.twiddleTable = TWIDDLE_TABLE;

    // Perform the last stage split operation to obtain N/2 complex FFT results.
    msp_split_q15(&paramsSplit, fft_array_copy);

    // Remove DC component.
    fft_array_copy[0] = 0;

    // Push array computing absolute value.
    for (i = 0; i < N_SAMPLES; i++) {
        if (fft_array_copy[i] >= 0) {
            __SET(fft_array[i]) = fft_array_copy[i];
        } else {
            __SET(fft_array[i]) = -fft_array_copy[i];
        }
    }

    NEXT(4);
}

TASK(task_done){
    if(__GET(iteration)>100) //10000
    {
        NEXT(0);
    }else{
        NEXT(1);
    }

}

void _benchmark_fft_init(){
        __THREAD(0);

        TASK_INIT(0, task_init, 0, 3);
        //TASK_INIT(0, task_sample,         0,      3);   //0 - [0,205]
        TASK_INIT(0, task_fft_prologue,   0,      263);   //1 - [0,203]
        //TASK_INIT(0, task_outer_loop,   300,    303);   //2 - [200,203]
        //TASK_INIT(0, task_fft_stage_1,       0,      3);     //3 - [0,0]
        TASK_INIT(0, task_fft_stage_2,       0,      263);     //3 - [0,0]
        //TASK_INIT(0, task_fft_stage_3,       0,      3);     //3 - [0,0]
        TASK_INIT(0, task_fft_epilogue,       0,      263);     //3 - [0,0]
        TASK_INIT(0, task_done,       0,      3);     //3 - [0,0]
}

/*
 * Abstracted helper function to check for overflow.
 */
static inline void msp_cmplx_overflow_q15(int16_t *src, bool *overflow)
{
    if ((CMPLX_REAL(src) > MSP_OVERFLOW_MAX) || (CMPLX_REAL(src) < MSP_OVERFLOW_MIN)) {
        *overflow = true;
    }
    if ((CMPLX_IMAG(src) > MSP_OVERFLOW_MAX) || (CMPLX_IMAG(src) < MSP_OVERFLOW_MIN)) {
        *overflow = true;
    }
}

/*
 * Abstracted helper functions for a radix-2 butterfly operation. The following
 * operation is performed at each stage:
 *     A = A + coeff*B
 *     B = A - coeff*B
 *
 * If overflow is detected the result is scaled by two:
 *     A = A/2
 *     B = A/2
 */
static inline void msp_cmplx_btfly_auto_q15(int16_t *srcA, int16_t *srcB, const _q15 *coeff, bool *scale, bool *overflow)
{
    /* Load coefficients. */
    _q15 tempR = CMPLX_REAL(coeff);
    _q15 tempI = CMPLX_IMAG(coeff);

    /* Calculate real and imaginary parts of coeff*B. */
    __q15cmpy(&tempR, &tempI, &CMPLX_REAL(srcB), &CMPLX_IMAG(srcB));

    /* Scale result if necessary. */
    if (*scale) {
        /* B = (A - coeff*B)/2 */
        CMPLX_REAL(srcB) = (CMPLX_REAL(srcA) - tempR) >> 1;
        CMPLX_IMAG(srcB) = (CMPLX_IMAG(srcA) - tempI) >> 1;

        /* A = (A + coeff*B)/2 */
        CMPLX_REAL(srcA) = (CMPLX_REAL(srcA) + tempR) >> 1;
        CMPLX_IMAG(srcA) = (CMPLX_IMAG(srcA) + tempI) >> 1;
    }
    else {
        /* B = A - coeff*B */
        CMPLX_REAL(srcB) = CMPLX_REAL(srcA) - tempR;
        CMPLX_IMAG(srcB) = CMPLX_IMAG(srcA) - tempI;

        /* A = A + coeff*B */
        CMPLX_REAL(srcA) = CMPLX_REAL(srcA) + tempR;
        CMPLX_IMAG(srcA) = CMPLX_IMAG(srcA) + tempI;
    }

    /* Check for possibility of overflow. */
    if (!*overflow) {
        msp_cmplx_overflow_q15(srcA, overflow);
        msp_cmplx_overflow_q15(srcB, overflow);
    }
}

/*
 * Simplified radix-2 butterfly operation for e^(-2*pi*(0/4)). This abstracted
 * helper function takes advantage of the fact the the twiddle coefficients are
 * positive and negative one for a multiplication by e^(-2*pi*(0/4)). The
 * following operation is performed at each stage:
 *     A = A + (1+0j)*B
 *     B = A - (1+0j)*B
 *
 * If overflow is detected the result is scaled by two:
 *     A = A/2
 *     B = A/2
 */
static inline void msp_cmplx_btfly_c0_auto_q15(int16_t *srcA, int16_t *srcB, bool *scale, bool *overflow)
{
    int16_t tempR = CMPLX_REAL(srcB);
    int16_t tempI = CMPLX_IMAG(srcB);

    /* Scale result if necessary. */
    if (*scale) {
        /* B = (A - (1+0j)*B)/2 */
        CMPLX_REAL(srcB) = (CMPLX_REAL(srcA) - tempR) >> 1;
        CMPLX_IMAG(srcB) = (CMPLX_IMAG(srcA) - tempI) >> 1;

        /* A = (A + (1+0j)*B)/2 */
        CMPLX_REAL(srcA) = (CMPLX_REAL(srcA) + tempR) >> 1;
        CMPLX_IMAG(srcA) = (CMPLX_IMAG(srcA) + tempI) >> 1;
    }
    else {
        /* B = A - coeff*B */
        CMPLX_REAL(srcB) = CMPLX_REAL(srcA) - tempR;
        CMPLX_IMAG(srcB) = CMPLX_IMAG(srcA) - tempI;

        /* A = A + coeff*B */
        CMPLX_REAL(srcA) = CMPLX_REAL(srcA) + tempR;
        CMPLX_IMAG(srcA) = CMPLX_IMAG(srcA) + tempI;
    }

    /* Check for possibility of overflow. */
    if (!*overflow) {
        msp_cmplx_overflow_q15(srcA, overflow);
        msp_cmplx_overflow_q15(srcB, overflow);
    }
}

/*
 * Simplified radix-2 butterfly operation for e^(-2*pi*(1/4)). This abstracted
 * helper function takes advantage of the fact the the twiddle coefficients are
 * positive and negative one for a multiplication by e^(-2*pi*(1/4)).  The
 * following operation is performed at each stage:
 *     A = A + (0-1j)*B
 *     B = A - (0-1j)*B
 *
 * If overflow is detected the result is scaled by two:
 *     A = A/2
 *     B = A/2
 */
static inline void msp_cmplx_btfly_c1_auto_q15(int16_t *srcA, int16_t *srcB, bool *scale, bool *overflow)
{
    int16_t tempR = CMPLX_REAL(srcB);
    int16_t tempI = CMPLX_IMAG(srcB);

    /* Scale result if necessary. */
    if (*scale) {
        /* B = (A - (0-1j)*B)/2 */
        CMPLX_REAL(srcB) = (CMPLX_REAL(srcA) - tempI) >> 1;
        CMPLX_IMAG(srcB) = (CMPLX_IMAG(srcA) + tempR) >> 1;

        /* A = (A + (0-1j)*B)/2 */
        CMPLX_REAL(srcA) = (CMPLX_REAL(srcA) + tempI) >> 1;
        CMPLX_IMAG(srcA) = (CMPLX_IMAG(srcA) - tempR) >> 1;
    }
    else {
        /* B = A - coeff*B */
        CMPLX_REAL(srcB) = CMPLX_REAL(srcA) - tempI;
        CMPLX_IMAG(srcB) = CMPLX_IMAG(srcA) + tempR;

        /* A = A + coeff*B */
        CMPLX_REAL(srcA) = CMPLX_REAL(srcA) + tempI;
        CMPLX_IMAG(srcA) = CMPLX_IMAG(srcA) - tempR;
    }

    /* Check for possibility of overflow. */
    if (!*overflow) {
        msp_cmplx_overflow_q15(srcA, overflow);
        msp_cmplx_overflow_q15(srcB, overflow);
    }
}
