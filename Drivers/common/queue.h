#pragma once
#include <sys/types.h>

#define QUEUE_MAX_NUM 32    // キューに登録できるデータの最大数（適宜変更すること）

typedef enum{
    QUEUE_SUCCESS = 0,
    QUEUE_FAILED,
} queue_status_e;

queue_status_e queue_entry(u_int8_t *p_entry, u_int32_t element_size, u_int32_t mux_num, u_int32_t *p_id);
queue_status_e queue_enqueue(u_int8_t *p_entry, u_int8_t *pp_data, u_int32_t id);
queue_status_e queue_dequeue(u_int8_t *p_entry, u_int8_t *p_dst_buf, u_int32_t id);
queue_status_e queue_erase(u_int8_t *p_entry, u_int32_t id);