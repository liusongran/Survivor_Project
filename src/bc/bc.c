#include <task.h>
#include <apps.h>

#define SEED 4L
#define ITER 640
#define CHAR_BIT 8

static char bits[768] = {   //todo __nv
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,  /* 0   - 15  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 16  - 31  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 32  - 47  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 48  - 63  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 64  - 79  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 80  - 95  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 96  - 111 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 112 - 127 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 128 - 143 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 144 - 159 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 160 - 175 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 176 - 191 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 192 - 207 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 208 - 223 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 224 - 239 */
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,   /* 240 - 255 */
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,  /* 0   - 15  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 16  - 31  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 32  - 47  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 48  - 63  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 64  - 79  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 80  - 95  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 96  - 111 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 112 - 127 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 128 - 143 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 144 - 159 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 160 - 175 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 176 - 191 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 192 - 207 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 208 - 223 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 224 - 239 */
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,   /* 240 - 255 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 16  - 31  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 32  - 47  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 48  - 63  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 64  - 79  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 80  - 95  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 96  - 111 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 112 - 127 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 128 - 143 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 144 - 159 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 160 - 175 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 176 - 191 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 192 - 207 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 208 - 223 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 224 - 239 */
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/**
 * 1. TEB declaration here.
 */
TASK(init);                 //-(0)
TASK(select_func);          //-(1)
TASK(bit_count);            //-(2)
TASK(bitcount);             //-(3)
TASK(ntbl_bitcnt);          //-(4)
TASK(ntbl_bitcount);        //-(5)
TASK(BW_btbl_bitcount);     //-(6)
TASK(AR_btbl_bitcount);     //-(7)
TASK(bit_shifter);          //-(8)
TASK(end);                  //-(9)

/**
 * 2. Shared variable declaration here. (22 bytes)
 */
__shared(
        unsigned _v_n_0;            //-[1]:2
        unsigned _v_n_1;            //-[2]:2
        unsigned _v_n_2;            //-[3]:2
        unsigned _v_n_3;            //-[4]:2
        unsigned _v_n_4;            //-[5]:2
        unsigned _v_n_5;            //-[6]:2
        unsigned _v_n_6;            //-[7]:2
        unsigned _v_func;           //-[8]:2 --16
        uint32_t _v_seed;           //-[9]:4
        uint64_t _v_iter;           //-[10]:8 --28
)

//TASK(init) NOTE: [R-or-RW() || RW-or-W-or-WR(1,2,3,4,5,6,7,8)]
//TASK(select_func) NOTE: [R-or-RW(8) || RW-or-W-or-WR(8,9,10)]
//TASK(bit_count) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(1,9,10)]
//TASK(bitcount) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(2,9,10)]
//TASK(ntbl_bitcnt) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(3,9,10)]
//TASK(ntbl_bitcount) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(4,9,10)]
//TASK(BW_btbl_bitcount) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(5,9,10)]
//TASK(AR_btbl_bitcount) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(6,9,10)]
//TASK(bit_shifter) NOTE: [R-or-RW(9,10) || RW-or-W-or-WR(7,9,10)]
//TASK(end) NOTE: [R-or-RW() || RW-or-W-or-WR()]


/**
 * 3. TASK definition here.
 */
TASK(init) { //-->0, NOTE: R() || W(1,2,3,4,5,6,7,8)
    __SET(_v_func) = 0;
    __SET(_v_n_0) = 0;
    __SET(_v_n_1) = 0;
    __SET(_v_n_2) = 0;
    __SET(_v_n_3) = 0;
    __SET(_v_n_4) = 0;
    __SET(_v_n_5) = 0;
    __SET(_v_n_6) = 0;

    NEXT(1);
}

TASK(select_func) { //-->1, NOTE: R(8) || W(8,9,10)
    __SET(_v_seed) = (uint32_t) SEED; // for testing, seed is always the same
    __SET(_v_iter) = 0;
    if (__GET(_v_func) == 0) {
        __SET(_v_func)++;
        NEXT(2);
    }
    else if (__GET(_v_func) == 1) {
        __SET(_v_func)++;
        NEXT(3);
    }
    else if (__GET(_v_func) == 2) {
        __SET(_v_func)++;
        NEXT(4);
    }
    else if (__GET(_v_func) == 3) {
        __SET(_v_func)++;
        NEXT(5);
    }
    else if (__GET(_v_func) == 4) {
        __SET(_v_func)++;
        NEXT(6);
    }
    else if (__GET(_v_func) == 5) {
        __SET(_v_func)++;
        NEXT(7);
    }
    else if (__GET(_v_func) == 6) {
        __SET(_v_func)++;
        NEXT(8);
    }
    else {
        NEXT(9);
    }
}

TASK(bit_count) { //-->2, NOTE: R(9,10) || W(1,9,10)
    uint16_t i;
    uint32_t tmp_seed;
    unsigned temp = 0;
    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        tmp_seed = __GET(_v_seed);
        __SET(_v_seed) = tmp_seed + 13;
        if (tmp_seed) do
                temp++;
            while (0 != (tmp_seed = tmp_seed & (tmp_seed - 1)));
        __SET(_v_n_0) += temp;
        temp = 0;
    }
    __SET(_v_iter) += (ITER >> 4);

    if (__GET(_v_iter) < ITER) {
        NEXT(2);
    }
    else {
        NEXT(1);
    }
}

TASK(bitcount) { //-->3, NOTE: R(9,10) || W(2,9,10)
    uint16_t i;
    uint32_t tmp_seed;
    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        tmp_seed = __GET(_v_seed);
        __SET(_v_seed) = tmp_seed + 13;
        tmp_seed = ((tmp_seed & 0xAAAAAAAAL) >>  1) + (tmp_seed & 0x55555555L);
        tmp_seed = ((tmp_seed & 0xCCCCCCCCL) >>  2) + (tmp_seed & 0x33333333L);
        tmp_seed = ((tmp_seed & 0xF0F0F0F0L) >>  4) + (tmp_seed & 0x0F0F0F0FL);
        tmp_seed = ((tmp_seed & 0xFF00FF00L) >>  8) + (tmp_seed & 0x00FF00FFL);
        tmp_seed = ((tmp_seed & 0xFFFF0000L) >> 16) + (tmp_seed & 0x0000FFFFL);
        __SET(_v_n_1) += (int)tmp_seed;
    }
    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(3);
    }
    else {
        NEXT(1);
    }
}

int recursive_cnt(uint32_t x) {
    int cnt = bits[(int)(x & 0x0000000FL)];

    if (0L != (x >>= 4))
        cnt += recursive_cnt(x);

    return cnt;
}

int non_recursive_cnt(uint32_t x) {
    int cnt = bits[(int)(x & 0x0000000FL)];

    while (0L != (x >>= 4)) {
        cnt += bits[(int)(x & 0x0000000FL)];
    }

    return cnt;
}

TASK(ntbl_bitcnt) { //-->4, NOTE: R(9,10) || W(3,9,10)
    // TRICK ALERT!
    uint16_t i;
    uint32_t tmp_seed;

    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        tmp_seed = __GET(_v_seed);
        __SET(_v_n_2) += non_recursive_cnt(tmp_seed);
        __SET(_v_seed) = tmp_seed + 13;
    }
    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(4);
    }
    else {
        NEXT(1);
    }
}

TASK(ntbl_bitcount) { //-->5, NOTE: R(9,10) || W(4,9,10)
    // TRICK ALERT!
    uint16_t i, __cry;
    uint32_t tmp_seed;

    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        __cry = __GET(_v_seed);
        __SET(_v_n_3) +=
            bits[ (int) (__cry & 0x0000000FUL)] +
            bits[ (int)((__cry & 0x000000F0UL) >> 4) ] +
            bits[ (int)((__cry & 0x00000F00UL) >> 8) ] +
            bits[ (int)((__cry & 0x0000F000UL) >> 12)] +
            bits[ (int)((__cry & 0x000F0000UL) >> 16)] +
            bits[ (int)((__cry & 0x00F00000UL) >> 20)] +
            bits[ (int)((__cry & 0x0F000000UL) >> 24)] +
            bits[ (int)((__cry & 0xF0000000UL) >> 28)];

        tmp_seed = __GET(_v_seed);
        __SET(_v_seed) = tmp_seed + 13;
    }
    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(5);
    }
    else {
        NEXT(1);
    }
}

TASK(BW_btbl_bitcount) { //-->6, NOTE: R(9,10) || W(5,9,10)
    union {
        unsigned char ch[4];
        long y;
    } U;
    uint16_t i;
    uint32_t tmp_seed;

    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        U.y = __GET(_v_seed);
        __SET(_v_n_4) += bits[ U.ch[0] ] + bits[ U.ch[1] ] +
                         bits[ U.ch[3] ] + bits[ U.ch[2] ];
        tmp_seed = __GET(_v_seed);
        __SET(_v_seed) = tmp_seed + 13;
    }
    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(6);
    }
    else {
        NEXT(1);
    }
}

TASK(AR_btbl_bitcount) { //-->7, NOTE: R(9,10) || W(6,9,10)
    uint16_t i;
    uint32_t tmp_seed;
    unsigned char * Ptr;
    int Accu;

    for (i = __GET(_v_iter); i < ((ITER >> 3) + __GET(_v_iter)); i++) {
        Ptr = (unsigned char *) &__GET(_v_seed);
        Accu  = bits[ *Ptr++ ];
        Accu += bits[ *Ptr++ ];
        Accu += bits[ *Ptr++ ];
        Accu += bits[ *Ptr ];
        __SET(_v_n_5) += Accu;

        tmp_seed = __GET(_v_seed);
        __SET(_v_seed) = tmp_seed + 13;
    }
    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(7);
    }
    else {
        NEXT(1);
    }
}

TASK(bit_shifter) { //-->8, NOTE: R(9,10) || W(7,9,10)
    unsigned i, j, nn;
    uint32_t tmp_seed;

    for (j = __GET(_v_iter); j < ((ITER >> 3) + __GET(_v_iter)); j++) {
        tmp_seed = __GET(_v_seed);
        for (i = nn = 0; tmp_seed && (i < (sizeof(long) * CHAR_BIT)); ++i, tmp_seed >>= 1)
            nn += (unsigned)(tmp_seed & 1L);
        __SET(_v_n_6) += nn;
        tmp_seed = __GET(_v_seed);  // TRICK ALERT!
        tmp_seed += 13;
        __SET(_v_seed) = tmp_seed;
    }

    __SET(_v_iter) += (ITER >> 4);
    if (__GET(_v_iter) < ITER) {
        NEXT(8);
    }
    else {
        NEXT(1);
    }
}

TASK(end) { //-->9, NOTE: R() || W()
    NEXT(0);
}

extern uint16_t nvInited;
extern buffer_idx_t elkBufIdx;
void _benchmark_bc_init(void) {
    if (!nvInited) {
        __THREAD(0);

        TASK_INIT(0,    init,               0,  15,  0,  15,  0,  0,  0,  15, 16);
        TASK_INIT(0,    select_func,       14,  27, 16,  27, 14, 15, 16,  27, 12);
        TASK_INIT(0,    bit_count,          2,  27,  2,  27, 16, 27,  2,  27, 26);
        TASK_INIT(0,    bitcount,           4,  27,  4,  27, 16, 27,  4,  27, 24);
        TASK_INIT(0,    ntbl_bitcnt,        6,  27,  6,  27, 16, 27,  6,  27, 22);
        TASK_INIT(0,    ntbl_bitcount,      8,  27,  8,  27, 16, 27,  8,  27, 20);
        TASK_INIT(0,    BW_btbl_bitcount,  10,  27, 10,  27, 16, 27, 10,  27, 18);
        TASK_INIT(0,    AR_btbl_bitcount,  12,  27, 12,  27, 16, 27, 12,  27, 16);
        TASK_INIT(0,    bit_shifter,       14,  27, 14,  27, 16, 27, 14,  27, 14);
        TASK_INIT(0,    end,                0,   0,  0,   0,  0,  0,  0,   0,  0);

//        TASK_INIT(0,    init,               0,  27);
//        TASK_INIT(0,    select_func,        0,  19);
//        TASK_INIT(0,    bit_count,          2,  19);
//        TASK_INIT(0,    bitcount,           8,  23);
//        TASK_INIT(0,    ntbl_bitcnt,        8,  21);
//        TASK_INIT(0,    ntbl_bitcount,      8,  25);
//        TASK_INIT(0,    BW_btbl_bitcount,   6,  19);
//        TASK_INIT(0,    AR_btbl_bitcount,   8,  27);
//        TASK_INIT(0,    bit_shifter,        4,  19);
//        TASK_INIT(0,    end,                0,  0);

    } else {
        __THREAD_DUMMY(0, elkBufIdx._idx);
    }
}
