#include <task.h>
#include <apps.h>

#define TASK_PRI    0

#if ENABLE_PRF
__nv uint8_t full_run_started = 0;
__nv uint8_t first_run = 1;
#endif

#ifndef RST_TIME
#define RST_TIME 25000
#endif

extern uint32_t lowestaddr;
extern uint32_t highestaddr;


#define NIL 0 // like NULL, but for indexes, not real pointers

#define DICT_SIZE         64
#define BLOCK_SIZE         32

#define NUM_LETTERS_IN_SAMPLE        2
#define LETTER_MASK             0x001F
#define LETTER_SIZE_BITS             8
#define NUM_LETTERS (LETTER_MASK + 1)


typedef uint16_t index_t;
typedef uint16_t letter_t;
typedef uint16_t sample_t;

// NOTE: can't use pointers, since need to ChSync, etc
typedef struct _node_t {
    letter_t letter; // 'letter' of the alphabet
    index_t sibling; // this node is a member of the parent's children list
    index_t child;   // link-list of children
} node_t;

// Tasks.
TASK(init);//0
TASK(init_dict);
TASK(sample);
TASK(measure_temp);
TASK(letterize);
TASK(compress);
TASK(find_sibling);
TASK(add_node);
//TASK(add_insert);
TASK(append_compressed);
TASK(done);  //10

__shared(   //3520
    node_t _v_compressed_data[BLOCK_SIZE];
    letter_t _v_letter;
    unsigned _v_letter_idx;
    sample_t _v_prev_sample;
    index_t _v_out_len;
    index_t _v_node_count;
    sample_t _v_sample;
    index_t _v_sample_count;
    index_t _v_sibling;
    index_t _v_child;
    index_t _v_parent;
    index_t _v_parent_next;
    node_t _v_parent_node;
    node_t _v_sibling_node;
    index_t _v_symbol;
    node_t _v_dict[DICT_SIZE];
)


static sample_t acquire_sample(letter_t prev_sample){
    letter_t sample = (prev_sample + 1) & 0x03;
    volatile uint32_t tempUsed = sample;
    tempUsed = tempUsed*tempUsed;
    tempUsed = sqrt(tempUsed);
    return sample;
}


TASK(init)//0
{
    __SET(_v_parent_next) = 0;
    __SET(_v_out_len) = 0;
    __SET(_v_letter) = 0;
    __SET(_v_prev_sample) = 0;
    __SET(_v_letter_idx) = 0;
    __SET(_v_sample_count) = 1;

    uint16_t j=__GET(_v_letter);;
    for(j=0;j<NUM_LETTERS;j++){
        //j = __GET(_v_letter);
        __SET(_v_dict[j].letter) = j ;
        __SET(_v_dict[j].sibling) =  NIL;
        __SET(_v_dict[j].child) = NIL;
        __SET(_v_letter)++;
    }
    __SET(_v_node_count) =  NUM_LETTERS;
    NEXT(1);
}

TASK(sample)//1
{
    unsigned next_letter_idx = __GET(_v_letter_idx) + 1;
    sample_t prev_sample;
    prev_sample = __GET(_v_prev_sample);
    

    if (next_letter_idx == NUM_LETTERS_IN_SAMPLE)
        next_letter_idx = 0;

    if (__GET(_v_letter_idx) == 0) {
        __SET(_v_letter_idx) = next_letter_idx;
        sample_t sample = acquire_sample(prev_sample);
        prev_sample = sample;
        __SET(_v_prev_sample) = prev_sample;
        __SET(_v_sample) = sample;
    } else {
        __SET(_v_letter_idx) = next_letter_idx;
    }
    NEXT(2);
}

TASK(letterize)//2
{
    unsigned letter_idx = __GET(_v_letter_idx);
    if (letter_idx == 0)
        letter_idx = NUM_LETTERS_IN_SAMPLE;
    else
        letter_idx--;

    unsigned letter_shift = LETTER_SIZE_BITS * letter_idx;
    letter_t letter = (__GET(_v_sample) & (LETTER_MASK << letter_shift)) >> letter_shift;

    __SET(_v_letter) = letter;
    index_t parent = __GET(_v_parent_next);
    uint16_t __cry;
    uint8_t testFlag=0;
    __cry = __GET(_v_dict[parent].child);
    __SET(_v_sibling) = __cry ;
    __cry = __GET(_v_dict[parent].letter);
    __SET(_v_parent_node.letter) =  __cry;
    __cry = __GET(_v_dict[parent].sibling);
    __SET(_v_parent_node.sibling) = __cry;
    __cry = __GET(_v_dict[parent].child);
    __SET(_v_parent_node.child) = __cry;
    __SET(_v_parent) = parent;
    __cry = __GET(_v_dict[parent].child);
    __SET(_v_child) = __cry;
    __SET(_v_sample_count)++;
    uint8_t tskIdx = 0;
    index_t starting_node_idx;
    index_t next_sibling;
    while(1){
        if (__GET(_v_sibling) != NIL) {
            int i = __GET(_v_sibling);
            uint16_t __cry = __GET(_v_letter);
            if (__GET(_v_dict[i].letter) == __cry ) {
                __cry = __GET(_v_sibling);
                __SET(_v_parent_next) = __cry;
                NEXT(2);
            }else{
                if(__GET(_v_dict[i].sibling) != 0){
                    __cry = __GET(_v_dict[i].sibling);
                    __SET(_v_sibling) = __cry;
                }else{}
                    starting_node_idx = (index_t)__GET(_v_letter);
                    __SET(_v_parent_next) = starting_node_idx;

                    if (__GET(_v_child) == NIL) {
                        //NEXT(3);
                        if (__GET(_v_node_count) == DICT_SIZE) { // wipe the table if full
        while (1);
    }

    index_t child = __GET(_v_node_count);
    uint16_t __cry;
    if (__GET(_v_parent_node.child) == NIL) { // the only child

        int i = __GET(_v_parent);

        __cry = __GET(_v_parent_node.letter);
        __SET(_v_dict[i].letter) = __cry;
        __cry  = __GET(_v_parent_node.sibling);
        __SET(_v_dict[i].sibling) = __cry;
        __cry = child;
        __SET(_v_dict[i].child) = __cry;

    } else { // a sibling
        index_t last_sibling = __GET(_v_sibling);
        __cry = __GET(_v_sibling_node.letter);
        __SET(_v_dict[last_sibling].letter) = __cry;
        __cry = child;
        __SET(_v_dict[last_sibling].sibling) = __cry;
        __cry  = __GET(_v_sibling_node.child);
        __SET(_v_dict[last_sibling].child) = __cry;
    }
    __cry = __GET(_v_letter);
    __SET(_v_dict[child].letter) = __cry;
    __SET(_v_dict[child].sibling) = NIL;
    __SET(_v_dict[child].child) = NIL;
    __cry = __GET(_v_parent);
    __SET(_v_symbol) = __cry;
    __SET(_v_node_count)++;

    //NEXT(5);
    //uint16_t __cry;
    int i = __GET(_v_out_len);
    __cry = __GET(_v_symbol);
    __SET(_v_compressed_data[i].letter) = __cry;
    __SET(_v_out_len)++;

    if ( (__GET(_v_out_len)) == BLOCK_SIZE) {
        NEXT(3);
    } else {
        NEXT(1);
    }
                    } else {
                        
                        while(1){
                            i = __GET(_v_sibling);
                            if(__GET(_v_dict[i].sibling) != NIL){
                                next_sibling = __GET(_v_dict[i].sibling);
                                __SET(_v_sibling) = next_sibling;
                            }else{
                                break;
                            }
                        }
                        __cry = __GET(_v_dict[i].letter);
                        __SET(_v_sibling_node.letter) = __cry;
                        __cry = __GET(_v_dict[i].sibling);
                        __SET(_v_sibling_node.sibling) = __cry;
                        __cry = __GET(_v_dict[i].child);
                        __SET(_v_sibling_node.child) = __cry;
                        //NEXT(3);
                        if (__GET(_v_node_count) == DICT_SIZE) { // wipe the table if full
        while (1);
    }

    index_t child = __GET(_v_node_count);
    uint16_t __cry;
    if (__GET(_v_parent_node.child) == NIL) { // the only child

        int i = __GET(_v_parent);

        __cry = __GET(_v_parent_node.letter);
        __SET(_v_dict[i].letter) = __cry;
        __cry  = __GET(_v_parent_node.sibling);
        __SET(_v_dict[i].sibling) = __cry;
        __cry = child;
        __SET(_v_dict[i].child) = __cry;

    } else { // a sibling
        index_t last_sibling = __GET(_v_sibling);
        __cry = __GET(_v_sibling_node.letter);
        __SET(_v_dict[last_sibling].letter) = __cry;
        __cry = child;
        __SET(_v_dict[last_sibling].sibling) = __cry;
        __cry  = __GET(_v_sibling_node.child);
        __SET(_v_dict[last_sibling].child) = __cry;
    }
    __cry = __GET(_v_letter);
    __SET(_v_dict[child].letter) = __cry;
    __SET(_v_dict[child].sibling) = NIL;
    __SET(_v_dict[child].child) = NIL;
    __cry = __GET(_v_parent);
    __SET(_v_symbol) = __cry;
    __SET(_v_node_count)++;

    //NEXT(5);
    //uint16_t __cry;
    int i = __GET(_v_out_len);
    __cry = __GET(_v_symbol);
    __SET(_v_compressed_data[i].letter) = __cry;
    __SET(_v_out_len)++;

    if ( (__GET(_v_out_len)) == BLOCK_SIZE) {
        NEXT(3);
    } else {
        NEXT(1);
    }

                    }
            }
        }else{
            break;
        }
    }
    starting_node_idx = (index_t)__GET(_v_letter);
    __SET(_v_parent_next) = starting_node_idx;

    if (__GET(_v_child) == NIL) {
        //NEXT(3);
        if (__GET(_v_node_count) == DICT_SIZE) { // wipe the table if full
        while (1);
    }

    index_t child = __GET(_v_node_count);
    uint16_t __cry;
    if (__GET(_v_parent_node.child) == NIL) { // the only child

        int i = __GET(_v_parent);

        __cry = __GET(_v_parent_node.letter);
        __SET(_v_dict[i].letter) = __cry;
        __cry  = __GET(_v_parent_node.sibling);
        __SET(_v_dict[i].sibling) = __cry;
        __cry = child;
        __SET(_v_dict[i].child) = __cry;

    } else { // a sibling
        index_t last_sibling = __GET(_v_sibling);
        __cry = __GET(_v_sibling_node.letter);
        __SET(_v_dict[last_sibling].letter) = __cry;
        __cry = child;
        __SET(_v_dict[last_sibling].sibling) = __cry;
        __cry  = __GET(_v_sibling_node.child);
        __SET(_v_dict[last_sibling].child) = __cry;
    }
    __cry = __GET(_v_letter);
    __SET(_v_dict[child].letter) = __cry;
    __SET(_v_dict[child].sibling) = NIL;
    __SET(_v_dict[child].child) = NIL;
    __cry = __GET(_v_parent);
    __SET(_v_symbol) = __cry;
    __SET(_v_node_count)++;

    //NEXT(5);
    //uint16_t __cry;
    int i = __GET(_v_out_len);
    __cry = __GET(_v_symbol);
    __SET(_v_compressed_data[i].letter) = __cry;
    __SET(_v_out_len)++;

    if ( (__GET(_v_out_len)) == BLOCK_SIZE) {
        NEXT(3);
    } else {
        NEXT(1);
    }
    }
    else {
        
        int i;
                        while(1){
                            i = __GET(_v_sibling);
                            if(__GET(_v_dict[i].sibling) != NIL){
                                next_sibling = __GET(_v_dict[i].sibling);
                                __SET(_v_sibling) = next_sibling;
                            }else{
                                break;
                            }
                        }
                        __cry = __GET(_v_dict[i].letter);
                        __SET(_v_sibling_node.letter) = __cry;
                        __cry = __GET(_v_dict[i].sibling);
                        __SET(_v_sibling_node.sibling) = __cry;
                        __cry = __GET(_v_dict[i].child);
                        __SET(_v_sibling_node.child) = __cry;
                        //NEXT(3);
                        if (__GET(_v_node_count) == DICT_SIZE) { // wipe the table if full
        while (1);
    }

    index_t child = __GET(_v_node_count);
    uint16_t __cry;
    if (__GET(_v_parent_node.child) == NIL) { // the only child

        int i = __GET(_v_parent);

        __cry = __GET(_v_parent_node.letter);
        __SET(_v_dict[i].letter) = __cry;
        __cry  = __GET(_v_parent_node.sibling);
        __SET(_v_dict[i].sibling) = __cry;
        __cry = child;
        __SET(_v_dict[i].child) = __cry;

    } else { // a sibling
        index_t last_sibling = __GET(_v_sibling);
        __cry = __GET(_v_sibling_node.letter);
        __SET(_v_dict[last_sibling].letter) = __cry;
        __cry = child;
        __SET(_v_dict[last_sibling].sibling) = __cry;
        __cry  = __GET(_v_sibling_node.child);
        __SET(_v_dict[last_sibling].child) = __cry;
    }
    __cry = __GET(_v_letter);
    __SET(_v_dict[child].letter) = __cry;
    __SET(_v_dict[child].sibling) = NIL;
    __SET(_v_dict[child].child) = NIL;
    __cry = __GET(_v_parent);
    __SET(_v_symbol) = __cry;
    __SET(_v_node_count)++;

    //NEXT(5);
    //uint16_t __cry;
i = __GET(_v_out_len);
    __cry = __GET(_v_symbol);
    __SET(_v_compressed_data[i].letter) = __cry;
    __SET(_v_out_len)++;

    if ( (__GET(_v_out_len)) == BLOCK_SIZE) {
        NEXT(3);
    } else {
        NEXT(1);
    }
    }
}

/*
TASK(add_node)//3
{
    int i = __GET(_v_sibling);

    if (__GET(_v_dict[i].sibling) != NIL) {
        index_t next_sibling = __GET(_v_dict[i].sibling);
        __SET(_v_sibling) = next_sibling;
        NEXT(3);

    } else { // found last sibling in the list

        uint16_t __cry;

        __cry = __GET(_v_dict[i].letter);
        __SET(_v_sibling_node.letter) = __cry;
        __cry = __GET(_v_dict[i].sibling);
        __SET(_v_sibling_node.sibling) = __cry;
        __cry = __GET(_v_dict[i].child);
        __SET(_v_sibling_node.child) = __cry;

        NEXT(4);
    }
}*/

/*
TASK(add_insert)//3
{
    if (__GET(_v_node_count) == DICT_SIZE) { // wipe the table if full
        while (1);
    }

    index_t child = __GET(_v_node_count);
    uint16_t __cry;
    if (__GET(_v_parent_node.child) == NIL) { // the only child

        int i = __GET(_v_parent);

        __cry = __GET(_v_parent_node.letter);
        __SET(_v_dict[i].letter) = __cry;
        __cry  = __GET(_v_parent_node.sibling);
        __SET(_v_dict[i].sibling) = __cry;
        __cry = child;
        __SET(_v_dict[i].child) = __cry;

    } else { // a sibling
        index_t last_sibling = __GET(_v_sibling);
        __cry = __GET(_v_sibling_node.letter);
        __SET(_v_dict[last_sibling].letter) = __cry;
        __cry = child;
        __SET(_v_dict[last_sibling].sibling) = __cry;
        __cry  = __GET(_v_sibling_node.child);
        __SET(_v_dict[last_sibling].child) = __cry;
    }
    __cry = __GET(_v_letter);
    __SET(_v_dict[child].letter) = __cry;
    __SET(_v_dict[child].sibling) = NIL;
    __SET(_v_dict[child].child) = NIL;
    __cry = __GET(_v_parent);
    __SET(_v_symbol) = __cry;
    __SET(_v_node_count)++;

    //NEXT(5);
    //uint16_t __cry;
    int i = __GET(_v_out_len);
    __cry = __GET(_v_symbol);
    __SET(_v_compressed_data[i].letter) = __cry;
    __SET(_v_out_len)++;

    if ( (__GET(_v_out_len)) == BLOCK_SIZE) {
        NEXT(4);
    } else {
        NEXT(1);
    }
}*/

TASK(done)//4
{
    NEXT(0);

}



void _benchmark_cem_init(void)
{
    __THREAD(0);
    //TASK_INIT(TASK_PRI,initTask,5,may_war_set_crc[0][0],may_war_set_crc[0][1],TASK_breaking_crc[0]);

    TASK_INIT(TASK_PRI,init,                 36,  121);
    //TASK_INIT(TASK_PRI,init_dict,            0,  1);
    TASK_INIT(TASK_PRI,sample,               0,  611);
    //TASK_INIT(TASK_PRI,measure_temp,         0,  1);
    TASK_INIT(TASK_PRI,letterize,            0,  611);
    //TASK_INIT(TASK_PRI,compress,             0,  1);
    //TASK_INIT(TASK_PRI,find_sibling, 0,  1);
    //TASK_INIT(TASK_PRI,add_node, 0,  1);
    //TASK_INIT(TASK_PRI,add_insert, 0,  1);
    //TASK_INIT(TASK_PRI,append_compressed, 0,  1);
    //TASK_INIT(TASK_PRI,print, 1,may_war_set_cem[10][0],may_war_set_cem[10][1],TASK_breaking_cem[10]);
    TASK_INIT(TASK_PRI,done, 0,  0);

}

