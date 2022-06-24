#include "task.h"

extern thread_t _threads[MAX_THREAD_NUM];

/**-----------------------------------------------------------------------------*/
/** Assigns a slot to a thread. Should be called ONLY at the first system boot. */
void __create_thread(uint8_t priority, void *data_org, void *data_temp, uint16_t size){
    _threads[priority].priority = priority;
    _threads[priority].buffer.buf[0] = data_org;
    _threads[priority].buffer.buf[1] = data_temp;
    _threads[priority].buffer.size = size;
    _threads[priority].idx_of_first_empty_task = 0;
}
/**-------------------*/
/** Init. a new TASK. */
// @para. addr_length: in word(2bytes)
void __init_task(uint8_t priority, void *task_entry, uint16_t start_used_offset, uint16_t end_used_offset, uint16_t start_cksum_offset, uint16_t end_cksum_offset, uint16_t start_verify_offset, uint16_t end_verify_offset, uint16_t start_backup_offset, uint16_t end_backup_offset, uint16_t backup_size){
    uint16_t temp_index;

    temp_index = _threads[priority].idx_of_first_empty_task;
    _threads[priority].task_array[temp_index].fun_entry = task_entry;
    _threads[priority].task_array[temp_index].task_idx = temp_index;

    _threads[priority].task_array[temp_index].ck_set.start_used_offset = start_cksum_offset;
    _threads[priority].task_array[temp_index].ck_set.end_used_offset = end_cksum_offset;

    _threads[priority].task_array[temp_index].ck_set.start_cksum_offset = start_cksum_offset;
    _threads[priority].task_array[temp_index].ck_set.end_cksum_offset = end_cksum_offset;
    
    _threads[priority].task_array[temp_index].ck_set.start_verify_offset = start_verify_offset;
    _threads[priority].task_array[temp_index].ck_set.end_verify_offset = end_verify_offset;

    _threads[priority].task_array[temp_index].ck_set.start_backup_offset = start_backup_offset;
    _threads[priority].task_array[temp_index].ck_set.end_backup_offset = end_backup_offset;

    _threads[priority].task_array[temp_index].ck_set.backup_size = backup_size;

    _threads[priority].idx_of_first_empty_task++;
}
