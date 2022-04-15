#ifndef LIST_H_
#define LIST_H_
#include <stdint.h>
//#define CRC32                   //use this as default. comment it to use CRC16.

#define MAX_SUB_CKSUM_NUM   8
#define MAX_CKSUM_TAB_NUM   128

typedef struct{
    uint16_t    intvlStart;
    uint16_t    intvlEnd;
    uint16_t    subCksum;
}list_node_t;

/** Double Linked List */
typedef struct dblist_t{
    uint16_t    prevNode;           // index of `stElkList[MAX_SUB_CKSUM_NUM]`
    uint16_t    nextNode;           // index of `stElkList[MAX_SUB_CKSUM_NUM]`
}dlist_t;

/** ELK_struc: 
 *  - List to record interval distribution, also act as header of list `stElkList[MAX_SUB_CKSUM_NUM]` 
 */
typedef struct{
    uint16_t    usedNodeNum;        // number of valid node in list `stElkList[MAX_SUB_CKSUM_NUM]`
    uint16_t    nextNode;           // point to the first valid node in list `stElkList[MAX_SUB_CKSUM_NUM]`
    dlist_t     stElkList[MAX_SUB_CKSUM_NUM];
}elk_list_t;

/** ELK_lib_fun:
 *  - Init. `elk_list`
 */
inline void _elk_listInit(elk_list_t *list){
    list->usedNodeNum = 0;
    list->nextNode = 99;
}

/** ELK_lib_fun:
 *  - Add first node into `elk_list` and `stElkList[MAX_SUB_CKSUM_NUM]`
 */
inline void _elk_listFirstAdd(elk_list_t *list, uint16_t nodeIdx){
    list->stElkList[nodeIdx].nextNode = 99;
    list->stElkList[nodeIdx].prevNode = 99;

    list->usedNodeNum = 1;
    list->nextNode = nodeIdx;
}
#endif
