#include <task.h>
#include "apps.h"



static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

struct adpcm_state {
    short   valprev;    /* Previous output value */
    char    index;      /* Index into stepsize table */
};

#define DATASIZE 128    /* Data block size */
#define DURATION 10     /* How many seconds to measure */
#define ENCODE_ITR  9000
#define DECODE_ITR  1000

__nv uint16_t pcmdata[DATASIZE] = {
                                    66,  77,   8,   3,  32,  55,   8,  11,  99,  65,
                                    25,  89,   3,  22,  25, 121,  11,  90,  74,   1,
                                    12,  39,  54,  20,  22,  43,  45,  90,  81,  40,
                                    121, 177,  50,  55,  32, 173, 164, 132,  17, 174,
                                    61, 186,  96,  44, 120, 181,  24, 134,  68, 167,
                                    8,  26, 144, 138, 133,  48,  80,  60,  39,   6,
                                    86, 126, 154,  42, 150, 113, 105,  52, 139, 175,
                                    58,  98,  31, 182,  74, 169,   4,  23, 157, 189,
                                    72, 130,   9,  19,  12,  67,   2,  16,  49,  57,
                                    69,  94, 145, 136,  99, 152, 198,  59, 153, 127,
                                    92,  13,  14, 160,  35, 194, 107,  89, 199, 155,
                                    163, 156,  93, 140, 111,  63,  15,  79,  22, 159,
                                    65,  78,  81, 122, 135, 180,  76,   3,  38, 102 ,
                                    121, 177,  50,  55,  32, 173, 164, 132,  17, 174,
                                    61, 186,  96,  44, 120, 181,  24, 134,  68, 167,
                                    8,  26, 144, 138, 133,  48,  80,  60,  39,   6
};

__nv uint16_t pcmdata1[DATASIZE] = {
                                    86, 126, 154,  42, 150, 113, 105,  52, 139, 175,
                                        58,  98,  31, 182,  74, 169,   4,  23, 157, 189,
                                        72, 130,   9,  19,  12,  67,   2,  16,  49,  57,
                                        69,  94, 145, 136,  99, 152, 198,  59, 153, 127,
                                        92,  13,  14, 160,  35, 194, 107,  89, 199, 155,
                                        163, 156,  93, 140, 111,  63,  15,  79,  22, 159,
                                        65,  78,  81, 122, 135, 180,  76,   3,  38, 102,
                                        3,   1,   4,   6,   9,   5,  10,   8,  16,  20,
                                        19,  40,  16,  17,   2,  41,  80, 100,   5,  89,
                                        66,  77,   8,   3,  32,  55,   8,  11,  99,  65,
                                        25,  89,   3,  22,  25, 121,  11,  90,  74,   1,
                                        12,  39,  54,  20,  22,  43,  45,  90,  81,  40,
                                        3,   1,   4,   6,   9,   5,  10,   8,  16,  20,
                                        19,  40,  16,  17,   2,  41,  80, 100,   5,  89,
                                        66,  77,   8,   3,  32,  55,   8,  11,  99,  65,
                                        25,  89,   3,  22,  25, 121,  11,  90,  74,   1,
                                        12,  39,  54,  20,  22,  43,  45,  90,  81,  40,
                                        3,   1,   4,   6,   9,   5,  10,   8,  16,  20,
                                        19,  40,  16,  17,   2,  41,  80, 100,   5,  89,
                                        66,  77,   8,   3,  32,  55,   8,  11,  99,  65,
                                        25,  89,   3,  22,  25, 121,  11,  90,  74,   1,
                                        12,  39,  54,  20,  22,  43,  45,  90,  81,  40,
                                        3,   1,   4,   6,   9,   5,  10,   8,  16,  20
};

//__shared(
//uint16_t time;
//struct adpcm_state coder_1_state, coder_2_state, decoder_state;
////short pcmdata[DATASIZE];
//char adpcmdata[DATASIZE / 2];
//short pcmdata_2[DATASIZE];
//)

__shared(
     uint16_t time;                        //[1]->2                --2
     struct adpcm_state coder_1_state;     //[2]->2                --4
     struct adpcm_state coder_2_state;     //[3]->2                --6
     struct adpcm_state decoder_state;     //[4]->2                --8
     char adpcmdata[DATASIZE / 2];         //[5]->1*DATASIZE/2=64  --72
     short pcmdata_2[DATASIZE];            //[6]->1*DATASIZE=128   --200
)

// TASK(init) [R-or-RW(6) || RW-or-W-or-WR(1)]
// TASK(coder) [R-or-RW(1,2,3,5) || RW-or-W-or-WR(1,3)]
// TASK(decoder) [R-or-RW(1,4,5,6) || RW-or-W-or-WR(4)]
TASK(init);
TASK(coder);
TASK(decoder);

TASK(init){
    __SET(time) = 0;
    int i;
    for (i=0;i<DATASIZE;i++){
        __GET(pcmdata_2[i])=0;
    }
    NEXT(1);

}

TASK(coder){
        uint16_t len = DATASIZE;
        uint16_t* inp;         /* Input buffer pointer */
        signed char* outp;      /* output buffer pointer */
        int val;            /* Current input sample value */
        int sign;           /* Current adpcm sign bit */
        int delta;          /* Current adpcm output value */
        int diff;           /* Difference between val and valprev */
        int step;           /* Stepsize */
        int valpred;        /* Predicted output value */
        int vpdiff;         /* Current change to valpred */
        int index;          /* Current step change index */
        int outputbuffer;       /* place to keep previous 4-bit value */
        int bufferstep;     /* toggle between outputbuffer/output */


        outp = (signed char*)__GET(adpcmdata);
        if(__GET(time)%2){
            inp = &pcmdata;
        }else{
            inp = &pcmdata1;
        }

        if( __GET(time) < 10){
            valpred = __GET(coder_1_state.valprev);
            index = __GET(coder_1_state.index);
        }
        else
        {
            valpred = __GET(coder_2_state.valprev);
            index = __GET(coder_2_state.index);
        }
        step = stepsizeTable[index];

        bufferstep = 1;

        for (; len > 0; len--) {
            val = (*inp)++;
            /* Step 1 - compute difference with previous value */
            diff = val - valpred;
            sign = (diff < 0) ? 8 : 0;
            if (sign) diff = (-diff);

            /* Step 2 - Divide and clamp */
            /* Note:
            ** This code *approximately* computes:
            **    delta = diff*4/step;
            **    vpdiff = (delta+0.5)*step/4;
            ** but in shift step bits are dropped. The net result of this is
            ** that even if you have fast mul/div hardware you cannot put it to
            ** good use since the fixup would be too expensive.
            */
            delta = 0;
            vpdiff = (step >> 3);

            if (diff >= step) {
                delta = 4;
                diff -= step;
                vpdiff += step;
            }
            step >>= 1;
            if (diff >= step) {
                delta |= 2;
                diff -= step;
                vpdiff += step;
            }
            step >>= 1;
            if (diff >= step) {
                delta |= 1;
                vpdiff += step;
            }

            /* Step 3 - Update previous value */
            if (sign)
                valpred -= vpdiff;
            else
                valpred += vpdiff;

            /* Step 4 - Clamp previous value to 16 bits */
            if (valpred > 32767)
                valpred = 32767;
            else if (valpred < -32768)
                valpred = -32768;

            /* Step 5 - Assemble value, update index and step values */
            delta |= sign;

            index += indexTable[delta];
            if (index < 0) index = 0;
            if (index > 88) index = 88;
            step = stepsizeTable[index];

            /* Step 6 - Output value */
            if (bufferstep) {
                outputbuffer = (delta << 4) & 0xf0;
            }
            else {
                *outp++ = (delta & 0x0f) | outputbuffer;
            }
            bufferstep = !bufferstep;
        }

        /* Output last step, if needed */
        if (!bufferstep)
            *outp++ = outputbuffer;
        if(__GET(time)<10){
        }
        else{
            __SET(coder_2_state.valprev) = valpred;
            __SET(coder_2_state.index) = index;
        }
        __SET(time)++;

        if(__GET(time)<ENCODE_ITR)
            NEXT(1);
        else if(__GET(time)<(ENCODE_ITR+DECODE_ITR))
        {
            NEXT(2);
        }
}


TASK(decoder) {
    signed char* inp;       /* Input buffer pointer */
    short* outp;        /* output buffer pointer */
    int sign;           /* Current adpcm sign bit */
    int delta;          /* Current adpcm output value */
    int step;           /* Stepsize */
    int valpred;        /* Predicted value */
    int vpdiff;         /* Current change to valpred */
    int index;          /* Current step change index */
    int inputbuffer;        /* place to keep next 4-bit value */
    int bufferstep;     /* toggle between inputbuffer/input */
    
outp = (short*)__GET(pcmdata_2);
inp = (signed char*)__GET(adpcmdata);
uint16_t len = DATASIZE;

valpred = __GET(decoder_state.valprev);
index = __GET(decoder_state.index);
step = stepsizeTable[index];

bufferstep = 0;

for (; len > 0; len--) {

    /* Step 1 - get the delta value */
    if (bufferstep) {
        delta = inputbuffer & 0xf;
    }
    else {
        inputbuffer = *inp++;
        delta = (inputbuffer >> 4) & 0xf;
    }
    bufferstep = !bufferstep;

    /* Step 2 - Find new index value (for later) */
    index += indexTable[delta];
    if (index < 0) index = 0;
    if (index > 88) index = 88;

    /* Step 3 - Separate sign and magnitude */
    sign = delta & 8;
    delta = delta & 7;

    /* Step 4 - Compute difference and new predicted value */
    /*
    ** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
    ** in adpcm_coder.
    */
    vpdiff = step >> 3;
    if (delta & 4) vpdiff += step;
    if (delta & 2) vpdiff += step >> 1;
    if (delta & 1) vpdiff += step >> 2;

    if (sign)
        valpred -= vpdiff;
    else
        valpred += vpdiff;

    /* Step 5 - clamp output value */
    if (valpred > 32767)
        valpred = 32767;
    else if (valpred < -32768)
        valpred = -32768;

    /* Step 6 - Update step value */
    step = stepsizeTable[index];

    /* Step 7 - Output value */
    *outp++ = valpred;
}
__SET(decoder_state.valprev) = valpred;
__SET(decoder_state.index) = index;
/*state->valprev = valpred;
state->index = index;*/

if(__GET(time)<(ENCODE_ITR+DECODE_ITR-1))
        NEXT(1);
else
{
        NEXT(0);
}

}
void _benchmark_adpcm_init()
{
    __THREAD(0);

    TASK_INIT(0, init,  0, 333);       //0
    TASK_INIT(0, coder,0,   77); //1
    TASK_INIT(0, decoder, 0, 333); //2
    //TEB_INIT( task_finish, 3, __GET_OFFSET(DMAflagStart));    //3
    //TEB_INIT( new_task, 1, may_war_set_sort[4][0], may_war_set_sort[4][1], teb_breaking_sort[4]);
}
