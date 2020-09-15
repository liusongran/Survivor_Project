#include <task.h>
#include <apps.h>
#include <math.h>
#include "msp-math.h"

#define MSG "Let's Go, Celtics!! Fighting for the NBA championship!!"
#define MSG_LEN 10

char * msgPt = MSG;

#define ROUND   2
/**
 * 1. TASK declaration here.
 */
TASK(initTask);      //0
TASK(ce_1);          //1
//TASK(ce_2);          //2
//TASK(is_i_prime);    //3
//TASK(ce_3);          //4
//TASK(cd);
TASK(encrypt_init);
//TASK(encrypt_finish);
TASK(decrypt_inner_loop);
TASK(decrypt_print);     //14

/**
 * 2. Shared variable declaration here.
 */
__shared(
    long int p;         //1
    long int q;         //2
    long int t;         //3
    long int k;         //4
    long int j;         //5
    long int i;         //6
    long int flag;      //7
    long int e[10];     //8
    long int d[10];     //9
    long int n;         //10
    long int m[10];     //11
    long int temp[10];  //12
    long int en[10];    //13
    long int en_pt;     //14
    long int en_ct;     //15
    long int en_key;    //16
    long int en_k;      //17
    long int en_cnt;    //18
    long int en_j;      //19
    long int de_pt;     //20
    long int de_ct;     //21
    long int de_key;    //22
    long int de_k;      //23
    long int de_cnt;    //24
    long int de_j;      //25
)

/**
 * 3. TASK definition here.
 */
TASK(initTask){//0   1~3
    int in_p = 19;
    int in_q = 23;
    int ii = 0;

    __SET(p)= in_p;
    __SET(q)= in_q;
    __SET(n)= in_p * in_q;
    __SET(t)= (in_p-1) * (in_q-1);
    __SET(i)=1;
    __SET(k)=0;
    __SET(flag)=0;
    for (ii = 0; ii < MSG_LEN; ii++) {
        __SET(m[ii]) = *(msgPt+ii);
    }
    NEXT(1);
}

TASK(ce_1){//1
    uint8_t tempFlag = 0;
    uint8_t roundCnter = 0;
    long int kk=1, __cry;
    int c;
    while(1){
        roundCnter++;
    while(1){
        __SET(i)++; // start with i=2
        if(__GET(i) >= __GET(t)){
            NEXT(2);
        }
        if(__GET(t) % __GET(i)){
            c=sqrt16(__GET(i));
            __SET(j) = c;
            for(c=2; c <= __GET( j) ;c++){
                if( __GET(i) % c==0){
                __SET(flag)=0;
                tempFlag = 1;
                }
            }
            if(tempFlag){
                tempFlag = 0;
                break;
            }
            __SET(flag) = 1;
            long int in_i = __GET(i);
            if( __GET(flag) == 1 && in_i != __GET(p) && in_i != __GET(q) ){
                __SET(e[__GET(k)]) = in_i ;
            } else {
                break;
            }
            while(1) {
                kk=kk +  __GET(t);
                if(kk % __GET( e[__GET(k)] ) ==0){
                    __cry = (kk/ __GET( e[ __GET(k) ]) );
                    __SET(flag) = __cry;
                    break;
                }
            }
            __cry = __GET(flag);
            if(__cry > 0){
                __SET(d[ __GET(k) ]) =__cry;
                __SET(k)++;
            }

            if (__GET(k) >= 9) {
                NEXT(2);
            }
        }
    }
    if(roundCnter>ROUND){
        NEXT(1);
    }
    }
}

TASK(encrypt_init){//2 
    long int __cry;
    __cry = __GET(m[ __GET(en_cnt) ]) ;
    __SET(en_pt) = __cry;
    __SET(en_pt) -=96;
    __SET(en_k)  = 1;
    __SET(en_j)  = 0;
    __cry = __GET(e[0]) ;
    __SET(en_key) = __cry;

    while(__GET(en_j) < __GET(en_key)){
        __cry = __GET(en_k) * __GET(en_pt);
        __SET(en_k) = __cry;
        __cry = __GET(en_k) % __GET(n);
        __SET(en_k) = __cry;
        __SET(en_j)++;
    }
    __cry = __GET(en_k);
    __SET(temp[ __GET(en_cnt) ]) = __cry;
    __cry = __GET(en_k) + 96;
    __SET(en_ct) = __cry;
    __cry = __GET(en_ct);
    __SET(en[ __GET(en_cnt) ]) = __cry;

    if (__GET(en_cnt) < MSG_LEN) {
        __SET(en_cnt)++;
        NEXT(2);
    } else {
        __SET(en[ __GET(en_cnt) ]) = -1;
        __SET(de_k)  = 1;
        __SET(de_j)  = 0;
        __cry =__GET(d[0]);
        __SET(de_key) = __cry;
        NEXT(3);
    }
}

TASK(decrypt_inner_loop){//3
    long int __cry;
    uint8_t tempFlag = 0;
    __cry =  __GET(temp[ __GET(de_cnt) ]);
    __SET(de_ct) = __cry;

    uint8_t tskIdx;
    uint8_t tmpIter = 30;   //TODO define it

    tskIdx = 3;
    while(tmpIter){
        tmpIter--;
        if(__GET(de_j) >= __GET(de_key)){
            __cry = __GET(de_k) + 96;
            __SET(de_pt) = __cry;
            __SET(m[ __GET(de_cnt) ]) = __cry;
            if (__GET(en[ __GET(de_cnt) ]) != -1) {
                __SET(de_cnt)++;
                tskIdx = 3;
                tempFlag = 1;
            } else {
                tskIdx = 4;
            }
            break;
        }
        __cry = __GET(de_k) * __GET(de_ct);
        __SET(de_k) = __cry;
        __cry = __GET(de_k) % __GET(n);
        __SET(de_k) = __cry;
        __SET(de_j)++;
    }
    if(tempFlag){
        tempFlag = 0;
        __cry = __GET(en_k);
        __SET(temp[ __GET(en_cnt) ]) = __cry;
        __cry = __GET(en_k) + 96;
        __SET(en_ct) = __cry;
        __cry = __GET(en_ct);
        __SET(en[ __GET(en_cnt) ]) = __cry;
        __SET(en[ __GET(en_cnt) ]) = -1;
        __SET(de_k)  = 1;
        __SET(de_j)  = 0;
        __cry =__GET(d[0]);
        __SET(de_key) = __cry;
    }
    NEXT(tskIdx);
}

TASK(decrypt_print){//5
    NEXT(0);
}

void _benchmark_rsa_init(){
    __THREAD(0);

    TASK_INIT(0, initTask,            0,    279);       //0
    TASK_INIT(0, ce_1,                0,   (0x6c)); 
    TASK_INIT(0, encrypt_init,        (0x44),   279);      
    TASK_INIT(0, decrypt_inner_loop,  (0x44),    279);    //8
    TASK_INIT(0, decrypt_print,       0,    0);     //10*/
}



