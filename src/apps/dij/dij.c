#include <task.h>
#include <apps.h>
#include "./dij.h"

#define N_NODES   25
#define Q_SIZE    2 * N_NODES

#define INFINITY  0xFFFF
#define UNDEFINED 0xFFFF

typedef struct {
    uint16_t dist;
    uint16_t prev;
} node_t;

typedef struct {
    uint16_t node;
    uint16_t dist;
    uint16_t prev;
} queue_t;

/**
 * 1. TASK declaration here.
 */
TASK(task_init);                    //-(0)
TASK(task_init_list);               //-(1)
//TASK(task_select_nearest_node);     //-(2)
TASK(task_find_shorter_path);       //-(3)
TASK(task_done);                    //-(4)

/**
 * 2. Shared variable declaration here. (714bytes)
 */
__shared(
    uint16_t deq_idx;               //-[2]:2
    uint16_t enq_idx;               //-[3]:2
    queue_t queue[Q_SIZE];          //-[1]:600  , 300
    uint16_t node_idx;              //-[4]:2
    uint16_t src_node;              //-[5]:2
    queue_t nearest_node;           //-[6]:6
    node_t node_list[N_NODES];      //-[7]:100  , 100 100
)

/**
 * 3. TASK definition here.
 */
TASK(task_init){//---0, NOTE: R(5) || W(1-P,2,3)
    __SET(deq_idx) = 0;
    __SET(enq_idx) = 0;

    // Enqueue.
    __SET(queue[0].node) = __GET(src_node);
    __SET(queue[0].dist) = 0;
    __SET(queue[0].prev) = UNDEFINED;
    ++__SET(enq_idx);

    NEXT(1);
}

TASK(task_init_list){//---1, NOTE: R(5) || W(5,7)
    uint16_t i, sn;
    uint16_t cost, node, dist, nearest_dist;

    for (i = 0; i < N_NODES; i++) {
        __SET(node_list[i].dist) = INFINITY;
        __SET(node_list[i].prev) = UNDEFINED;
    }

    sn = __GET(src_node);
    __SET(node_list[sn].dist) = 0;
    __SET(node_list[sn].prev) = UNDEFINED;

    sn++;
    // Test nodes 0, 1, 2, 3.
    if (sn < 4) {
        __SET(src_node) = 3;
    } else {
        __SET(src_node) = 0;
    }

    //NEXT(2);
    i = __GET(deq_idx);

    if (__GET(enq_idx) != i) {
        // Dequeue nearest node.
        __SET(nearest_node.node) = __GET(queue[i].node);
        __SET(nearest_node.dist) = __GET(queue[i].dist);
        __SET(nearest_node.prev) = __GET(queue[i].prev);
        i++;
        if (i < Q_SIZE) {
            __SET(deq_idx) = i;
        } else {
            __SET(deq_idx) = 0;
        }
        __SET(node_idx) = 0;

        node = __GET(nearest_node.node);

        for(i = __GET(node_idx);i < N_NODES; i++){
            cost = adj_matrix[node][i];
            if (cost != INFINITY) {
                nearest_dist = __GET(nearest_node.dist);
                dist = __GET(node_list[i].dist);
                if (dist == INFINITY || dist > (cost + nearest_dist)) {
                    __SET(node_list[i].dist) = nearest_dist + cost;
                    __SET(node_list[i].prev) = node;
                // Enqueue.
                    uint16_t j = __GET(enq_idx);
                    if (j == (__GET(deq_idx) - 1)) {
                //LOG("QUEUE IS FULL!");
                    }
                    __SET(queue[j].node) = i;
                    __SET(queue[j].dist) = nearest_dist + cost;
                    __SET(queue[j].prev) = node;
                    j++;
                    if (j < Q_SIZE) {
                        __SET(enq_idx) = j;
                    } else {
                        __SET(enq_idx) = 0;
                    }
                }
            }
        }
        __SET(node_idx) = N_NODES;
        NEXT(1);
    } else {
        NEXT(2);
    }
}
/*
TASK(task_select_nearest_node){//---2, NOTE: R(1-P,2,3) || W(2,4,6)

}*/


TASK(task_done){//---4
    NEXT(0);
}

/**
 * 0. Benchmark app Init here.
 */
extern uint16_t nvInited;
extern buffer_idx_t elkBufIdx;
void _benchmark_dijkstra_init(){
    if(!nvInited){
        __THREAD(0);

        TASK_INIT(0, task_init,                   0,    413);       //0
        TASK_INIT(0, task_init_list,              200,    399); //1
        //TASK_INIT(0, task_select_nearest_node,    0,    713); //2
        //TASK_INIT(0, task_find_shorter_path,      0,    99);    //3
        TASK_INIT(0, task_done,                   100,    413);  //4
    }else{
        __THREAD_DUMMY(0, elkBufIdx._idx);
    }
}



