
#include <stdint.h>
#include "apps.h"
#include <string.h>
#include "elk.h"

#define MAX_TASK_NUM        APP_TASK_NUM
#define PRIORITY_LEV        1
#define MAX_THREAD_NUM      PRIORITY_LEV

//#define __nv __attribute__((section(".TI.persistent")))

typedef struct {
    uint16_t nvCksumTemp;
    uint16_t svCksumTemp;
}cksum_temp_t;

typedef struct {
    volatile uint8_t idx;
    volatile uint8_t _idx;
}buffer_idx_t;

typedef struct {
    uint8_t itvalStartIdx;
    uint8_t itvalEndIdx;
}itval_idx_t;

/** Each thread will hold a buffer for the variables shared by the tasks it is encapsulating. */
typedef struct {
    void *buf[2];                               //hold pointers to global data.
    uint16_t size;                              //size of the buffer.
}buffer_t;

/** Var info for each Task. */
typedef struct {
    //uint16_t idxset_used_var;   //index of used vars for each task.
    //uint16_t idxset_wrt_var;    //index of write vars for each task.
    uint16_t start_used_offset;                 //start
    uint16_t end_used_offset;                   //end
}ck_set_t;

/** The main task structure for each Task. */
typedef struct {
    void *fun_entry;
    uint8_t task_idx;
    ck_set_t ck_set;
}task_t;

/** the main thread structure that holds all necessary info. */
typedef struct {
    uint8_t priority;                           //thread priority (unique)
    buffer_t buffer;                            //holds task shared persistent variables
    uint8_t idx_of_first_empty_task;
    task_t task_array[MAX_TASK_NUM];
}thread_t;

/** The task definition (single C function). */
typedef uint8_t (*taskfun_t) (buffer_t *);

/** Allocates a double buffer for the persistent variables in SRAM. */
#define __shared(...)   \
        typedef struct {    \
            __VA_ARGS__     \
        } SRAM_data_t  __attribute__ ((aligned (2)));    \
        __nv static SRAM_data_t __persistent_vars[2];        //TODO: glbal

/** Declare a TASK. */
#define TASK(name)  static uint8_t name(void *__buffer)

/** TASK structure init. */
void __init_task(uint8_t priority, void *task_entry, uint16_t start_used_offset, uint16_t end_used_offset);
#define TASK_INIT(priority, name, start_offset, end_offset) \
        __init_task(priority, (void *)&name, start_offset, end_offset)

/** Reads the value from the original stack. */
#define __GET(x) ((SRAM_data_t *)__buffer)->x

/** Writes the value to the temporary stack. */
#define __SET(x) ((SRAM_data_t *)__buffer)->x

/** Point to next TASK's ID. */
#define NEXT(id)  return (uint16_t)id

/** Creates a thread. */
void __create_thread(uint8_t priority, void *data_org, void *data_temp, uint16_t size);
#define __THREAD(priority)  \
        __create_thread(priority, (void *)&__persistent_vars[0], (void *)&__persistent_vars[1], sizeof(SRAM_data_t));  \
        memset((void *)&__persistent_vars[1], 0, sizeof(SRAM_data_t));\
        memset((void *)&__persistent_vars[0], 0, sizeof(SRAM_data_t))
        //memset((void *)&__persistent_vars[1], 0, sizeof(SRAM_data_t))

#define __THREAD_DUMMY(priority, idx)  \
        __create_thread(priority, (void *)&__persistent_vars[0], (void *)&__persistent_vars[1], sizeof(SRAM_data_t));  \
        memset((void *)&__persistent_vars[idx], 0, sizeof(SRAM_data_t));
