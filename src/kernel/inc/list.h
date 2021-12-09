#include <stdint.h>
//#define CRC32                   //use this as default. comment it to use CRC16.

#define MAX_SUB_CKSUM_NUM   4
#define MAX_CKSUM_TAB_NUM   128

typedef struct {
    uint16_t    intvlStart;
    uint16_t    intvlEnd;
    uint16_t    subCksum;
    uint16_t    paddingNum;     //number of padding-zeros to end
}list_node_t;

typedef struct dblist_t{
    //struct dblist_t *prev;
    uint16_t prevNode;
    //struct dblist_t *next;
    uint16_t nextNode;
    //uint16_t nodeIdx;
}dlist_t;

typedef struct {
    //dlist_t *prev;
    uint16_t prevNode;
    //dlist_t *next;
    uint16_t nextNode;
    dlist_t stElkList[MAX_SUB_CKSUM_NUM];
}elk_list_t;

inline void _elk_listInit(elk_list_t *list){
    //list->next = (dlist_t*)list;
    list->prevNode = 99;
    //list->prev = (dlist_t*)list;
    list->nextNode = 99;
    /*
    uint8_t tempItr;
    for (tempItr = 0; tempItr < MAX_SUB_CKSUM_NUM; tempItr++){
        list->stElkList[tempItr].nodeIdx = tempItr;
    }*/
}

inline void _elk_listFirstAdd(elk_list_t *list, uint16_t nodeIdx){
    //nextNode->prev = (dlist_t*)list;
    //nextNode->next = (dlist_t*)list;
    list->stElkList[nodeIdx].nextNode = 99;
    list->stElkList[nodeIdx].prevNode = 99;
    
    //list->next = nextNode;
    //list->prev = nextNode;
    list->nextNode = nodeIdx;
    list->prevNode = nodeIdx;
}
