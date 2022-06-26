#include <task.h>
#include <apps.h>
//#include <math.h>

#define NUM_BUCKETS 64// must be a power of 2
#define NUM_INSERTS (NUM_BUCKETS / 4) // shoot for 25% occupancy
#define NUM_LOOKUPS NUM_INSERTS
#define MAX_RELOCATIONS 8
#define BUFFER_SIZE 32
#define COUNTERNUM  35

typedef uint16_t value_t;
typedef uint16_t hash_t;
typedef uint16_t fingerprint_t;
typedef uint16_t index_t; // bucket index

typedef struct _insert_count {
    unsigned insert_count;
    unsigned inserted_count;
} insert_count_t;

typedef struct _lookup_count {
    unsigned lookup_count;
    unsigned member_count;
} lookup_count_t;

/**
 * 1. TEB declaration here.
 */
TASK(init);
TASK(generate_key);
//TASK(calc_indexes);
TASK(calc_indexes_index_1);
//TASK(calc_indexes_index_2);
//TASK(insert);
void * insert;
void * lookup;
void * calc_indexes_index_2;
void * relocate;
void * insert_done;
void * lookup_search;
void * lookup_done;
void * print_status;
TASK(add);
//TASK(relocate);
//TASK(insert_done);
//TASK(lookup);
//TASK(lookup_search);
//TASK(lookup_done);
//TASK(print_stats);
TASK(done);
// NOT USED.
//TASK(task_init_array);

/**
 * 2. Shared variable declaration here. (206 bytes)
 */

__shared(
    uint32_t _v_counter;                    //-[1]: 4
    value_t _v_key;                         //-[2]: 2
    void* _v_next_task;                     //-[3]: 4 --10
    fingerprint_t _v_fingerprint;           //-[4]: 2
    value_t _v_index1;                      //-[5]: 2
    value_t _v_index2;                      //-[6]: 2
    value_t _v_relocation_count;            //-[7]: 2
    value_t _v_insert_count;                //-[8]: 2 --20
    value_t _v_inserted_count;              //-[9]: 2
    value_t _v_lookup_count;                //-[10]: 2
    value_t _v_member_count;                //-[11]: 2 --26
    bool _v_success;                        //-[12]: 1
    bool _v_member;                         //-[13]: 1 --28
    fingerprint_t _v_filter[NUM_BUCKETS];   //-[14]: 2 * NUM_BUCKETS = 128  --256
)

//TASK(init) NOTE: [R-or-RW() || RW-or-W-or-WR(1,2,3,8,9,10,11,14)]
//TASK(generate_key) NOTE: [R-or-RW(2,3) || RW-or-W-or-WR(1,2,3,4)]
//TASK(calc_indexes_index_1) NOTE: [R-or-RW(2,3,4,11,13,14) || RW-or-W-or-WR(3,5,6,10,11,13)]
//TASK(add) NOTE: [R-or-RW(4,5,6,9,14) || RW-or-W-or-WR(2,3,4,5,7,8,9,12,14)]
//TASK(done) NOTE: [R-or-RW() || RW-or-W-or-WR()]

//void * insert;

static value_t init_key = 0x0001; // seeds the pseudo-random sequence of keys

static hash_t djb_hash(uint8_t* data, unsigned len){
    uint32_t hash = 15381;
    unsigned int i;
    for (i = 0; i < len; data++, i++)
        hash = ((sqrt(hash) << 5)*13 + hash)/27 + (*data);    //trick here.
        //hash = ((hash << 5) + hash) + (*data);
    return hash & 0xFFFF;
}

static index_t hash_to_index(fingerprint_t fp){
    hash_t hash = djb_hash((uint8_t *) &fp, sizeof(fingerprint_t));
    return hash & (NUM_BUCKETS - 1); // NUM_BUCKETS must be power of 2
}

static fingerprint_t hash_to_fingerprint(value_t key){
    return djb_hash((uint8_t *) &key, sizeof(value_t));
}


TASK(init){//-->0, NOTE: R() || W(1,3,4,9,10,11,12)
    unsigned i;
    for (i = 0; i < NUM_BUCKETS ; ++i) {
        __SET(_v_filter[i]) = 0;
    }

    __SET(_v_insert_count) = 0;
    __SET(_v_lookup_count) = 0;
    __SET(_v_inserted_count) = 0;
    __SET(_v_member_count) = 0;
    __SET(_v_key) = init_key;
    __SET(_v_next_task) = insert;
    __SET(_v_counter) = 0;

    NEXT(1);
}

TASK(generate_key){//-->2, 1 NOTE: R(3,4) || W(3)
    uint16_t __cry;

    __cry = (__GET(_v_key) + 1) * 17;
    __SET(_v_key) = __cry;

    void* next_task = __GET(_v_next_task);
    __SET(_v_counter)++;
    if(__GET(_v_counter)>COUNTERNUM){
        NEXT(4);
    }
    if (next_task == insert) {
        ////NEXT(6);
        __SET(_v_next_task) = add;
        ////NEXT(3);//task_calc_indexes
        ////uint16_t __cry;
        __cry = hash_to_fingerprint(__GET(_v_key));
        __SET(_v_fingerprint) = __cry;
        ////NEXT(4);
    } else if (next_task == lookup) {
        ////NEXT(10);
        __SET(_v_next_task) = lookup_search;
        ////NEXT(3);//task_calc_indexes
        //uint16_t __cry;
        __cry = hash_to_fingerprint(__GET(_v_key));
        __SET(_v_fingerprint) = __cry;
        ////NEXT(4);
    } else {
        while(1);
    }
    NEXT(2);
}

TASK(calc_indexes_index_1){//-->4, 2 NOTE: R(3) || W(6)
    uint16_t __cry;
    __cry = hash_to_index(__GET(_v_key));
    __SET(_v_index1) = __cry;

    ////NEXT(5);
    index_t fp_hash = hash_to_index(__GET(_v_fingerprint));
    ////uint16_t __cry;
    __cry = __GET(_v_index1) ^ fp_hash;
    __SET(_v_index2) = __cry;

    void* next_task = __GET(_v_next_task);

    if (next_task == add) {
        NEXT(3);//task_add
    } else if (next_task == lookup_search) {
        ////NEXT(11);//task_lookup_search
        if (__GET(_v_filter[__GET(_v_index1)]) == __GET(_v_fingerprint)) {
            __SET(_v_member) = true;
        } else {
            if (__GET(_v_filter[__GET(_v_index2)]) == __GET(_v_fingerprint)) {
                __SET(_v_member) = true;
            } else {
                __SET(_v_member) = false;
            }
        }
        ////NEXT(12);//lookup_done
        //uint16_t __cry;
        __SET(_v_lookup_count)++;
        __cry = __GET(_v_member_count) ;
        __cry+= __GET(_v_member);
        __SET(_v_member_count)  = __cry;
        if (__GET(_v_lookup_count) < NUM_LOOKUPS) {
            __SET(_v_next_task) = lookup;
            NEXT(1);
        } else {
            NEXT(4); //to end!!!!TODO:
        }
    } else {
        while(1); // Debugging pu__GETose
    }
}

TASK(add){//-->7, 3 NOTE: R(1-P,5,6,7) || W(1-P,5,6,8,13)
    uint16_t __cry;
    uint16_t __cry_idx = __GET(_v_index1);
    uint16_t __cry_idx2 = __GET(_v_index2);
    uint8_t tempFlag = 0;

    if (!__GET(_v_filter[__cry_idx])) {
        __SET(_v_success) = true;
        __cry = __GET(_v_fingerprint);
        __SET(_v_filter[__cry_idx]) = __cry;
        ////NEXT(9);//task_insert_done
        tempFlag = 1;
    } else {
        if (!__GET(_v_filter[__cry_idx2])) {
            __SET(_v_success) = true;
            __cry = __GET(_v_fingerprint);
            __SET(_v_filter[__cry_idx2])  = __cry;
            ////NEXT(9);//task_insert_done
            tempFlag = 1;
        } else { // evict one of the two entries
            fingerprint_t fp_victim;
            index_t index_victim;

            if (rand() % 2) {
                index_victim = __cry_idx;
                fp_victim = __GET(_v_filter[__cry_idx]);
            } else {
                index_victim = __cry_idx2;
                fp_victim = __GET(_v_filter[__cry_idx2]);
            }

            // Evict the victim
            __cry = __GET(_v_fingerprint);
            __SET(_v_filter[index_victim])  = __cry;
            __SET(_v_index1) = index_victim;
            __SET(_v_fingerprint) = fp_victim;
            __SET(_v_relocation_count) = 0;

            ////NEXT(8);//relocate dummy node
            //uint16_t __cry;
            fp_victim = __GET(_v_fingerprint);
            index_t fp_hash_victim = hash_to_index(fp_victim);
            index_t index2_victim = __GET(_v_index1) ^ fp_hash_victim;
            while(1){
                if (!__GET(_v_filter[index2_victim])) { // slot was free
                    __SET(_v_success) = true;
                    __SET(_v_filter[index2_victim]) = fp_victim;
                    ////NEXT(9);//task_insert_done
                    tempFlag = 1;
                    break;
                } else { // slot was occupied, rellocate the next victim
                    if (__GET(_v_relocation_count) >= MAX_RELOCATIONS) { // insert failed
                    __SET(_v_success) = false;
                    ////NEXT(9);//task_insert_done
                    tempFlag = 1;
                    break;
                    }
                __SET(_v_relocation_count)++;
                __SET(_v_index1) = index2_victim;
                __cry = __GET(_v_filter[index2_victim]);
                __SET(_v_fingerprint) = __cry;
                __SET(_v_filter[index2_victim]) = fp_victim;
                ////NEXT(8);//task_relocate
                }
            }
        }
    }
    if(tempFlag){
        __SET(_v_insert_count)++;
        __cry = __GET(_v_inserted_count);
        __cry+= __GET(_v_success);
        __SET(_v_inserted_count) = __cry;
        if (__GET(_v_insert_count) < NUM_INSERTS) {
            __SET(_v_next_task) = insert;
            NEXT(1);//task_generate_key
        } else {
            __SET(_v_next_task) = lookup;
            __SET(_v_key) = init_key;
            NEXT(1);//task_generate_key
        }
    }
}

TASK(done){//-->14, 4 NOTE: R() || W()
    NEXT(0);
}

void _benchmark_cuckoo_init(void){
    __THREAD(0);

    TASK_INIT(0,    init,                   0,      155);
    //TASK_INIT(0,    init_array,             0,      205);      //not used
    TASK_INIT(0,    generate_key,           0,      155);
    //TASK_INIT(0,    calc_indexes,           0,      205);
    TASK_INIT(0,    calc_indexes_index_1,   4,      155);
    //TASK_INIT(0,    calc_indexes_index_2,   0,      205);
    //TASK_INIT(0,    insert,                 0,      205);
    TASK_INIT(0,    add,                    4,      155);
    //TASK_INIT(0,    relocate,               0,      205);
    //TASK_INIT(0,    insert_done,            0,      205);
    //TASK_INIT(0,    lookup,                 0,      205);
    //TASK_INIT(0,    lookup_search,          0,      205);
    //TASK_INIT(0,    lookup_done,            0,      205);
    //TASK_INIT(0,    print_stats,            0,      205);
    TASK_INIT(0,    done,                   0,      0);
}
