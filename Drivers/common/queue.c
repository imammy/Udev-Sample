#include <stdio.h>
#include <string.h>
#include "queue.h"


typedef struct 
{
    u_int32_t *p_entry;    // 実態は、それぞれキューに登録したい側が持つ
    u_int32_t element_size;
    u_int32_t max_num;
    u_int32_t queue_head;
    u_int32_t queue_num;
} queue_t;


static queue_t s_queue[QUEUE_MAX_NUM];
// ↑のQueue自体の管理情報
static u_int32_t s_queue_head = 0;
static u_int32_t s_queue_num = 0;

queue_status_e queue_entry(u_int8_t *p_entry, u_int32_t element_size, u_int32_t max_num, u_int32_t *p_id) {
    if(s_queue_num >= QUEUE_MAX_NUM) {
        goto error_end;
    }

    

    s_queue[(s_queue_head + s_queue_num) % QUEUE_MAX_NUM].p_entry = (u_int32_t*)p_entry;
    s_queue[(s_queue_head + s_queue_num) % QUEUE_MAX_NUM].max_num = max_num;
    s_queue[(s_queue_head + s_queue_num) % QUEUE_MAX_NUM].element_size = element_size;
    s_queue[(s_queue_head + s_queue_num) % QUEUE_MAX_NUM].queue_head = 0;
    s_queue[(s_queue_head + s_queue_num) % QUEUE_MAX_NUM].queue_num = 0;
    *p_id = (s_queue_head + s_queue_num) % QUEUE_MAX_NUM;
    s_queue_num ++;

    printf("%02x entry\n", s_queue[*p_id].p_entry);
    printf("%d element_size\n", s_queue[*p_id].element_size);
    printf("%d max_num\n", s_queue[*p_id].max_num);
    return QUEUE_SUCCESS;

error_end:
    return QUEUE_FAILED;
}

queue_status_e queue_enqueue(u_int8_t *p_entry, u_int8_t *pp_data, u_int32_t id) {
    //printf("%02x enqueue\n", (u_int32_t*)p_entry);
    //printf("%d num \n", s_queue[id].queue_num);
    //printf("%d id \n", id);
    if(s_queue[id].p_entry != (u_int32_t*)p_entry) {
        goto error_end;
    }
    
    if(s_queue[id].queue_num >= s_queue[id].max_num) {
        goto error_end;
    }

    memcpy((u_int8_t*)s_queue[id].p_entry + (s_queue[id].element_size * ((s_queue[id].queue_head + s_queue[id].queue_num) % s_queue[id].max_num)),
           pp_data,
           s_queue[id].element_size);
    
    //for(int i = 0; i < s_queue[id].element_size; i++) {
    //    printf("%c", *(((u_int8_t*)s_queue[id].p_entry + (s_queue[id].element_size * ((s_queue[id].queue_head + s_queue[id].queue_num) % s_queue[id].max_num)) + i)));
    //}
    //printf("\n");
    s_queue[id].queue_num ++;

    return QUEUE_SUCCESS;

error_end:
    return QUEUE_FAILED;
}

queue_status_e queue_dequeue(u_int8_t *p_entry, u_int8_t *p_dst_buf, u_int32_t id) {
    if(s_queue[id].p_entry != (u_int32_t*)p_entry) {
        //printf("%02x entry error\n", (u_int32_t*)p_entry);
        //printf("%d num error\n", s_queue[id].queue_num);
        //printf("%d id \n", id);
        goto error_end;
    }

    if(s_queue[id].queue_num == 0) {
        goto error_end;
    }

    // printf("%d element size\n", s_queue[id].element_size);
    //printf("%d queue head\n", s_queue[id].queue_head);
    //printf("%d queue num\n", s_queue[id].queue_num);
    memcpy(p_dst_buf,
           (u_int8_t*)s_queue[id].p_entry + (s_queue[id].queue_head * s_queue[id].element_size),
           s_queue[id].element_size);
    
    //for(int i = 0; i < s_queue[id].element_size; i++) {
    //    printf("%c", *(((u_int8_t*)s_queue[id].p_entry + ((s_queue[id].queue_head * s_queue[id].element_size) + i))));
    //}
    //printf("\n");
    //for(int i = 0; i < s_queue[id].element_size; i++) {
    //    printf("%c", *(p_dst_buf+i));
    //}
    //printf("\n");

    // headは1進め、numは1減らす
    s_queue[id].queue_head = (s_queue[id].queue_head + 1) % s_queue[id].max_num;
    s_queue[id].queue_num --;

   return QUEUE_SUCCESS;

error_end:
    return QUEUE_FAILED;
}

queue_status_e queue_erase(u_int8_t *p_entry, u_int32_t id) {
    if(s_queue[id].p_entry != (u_int32_t*)p_entry) {
        goto error_end;
    }
    
    s_queue[id].p_entry = (u_int32_t*)0xFFFFFFFF;
    s_queue[id].max_num = 0;
    s_queue[id].element_size = 0;
    s_queue[id].queue_head = 0;
    s_queue[id].queue_head = 0;
    
    // headは1進め、numは1減らす
    s_queue_head = (s_queue_head + 1) % QUEUE_MAX_NUM;
    s_queue_num --;

    return QUEUE_SUCCESS;

error_end:
    return QUEUE_FAILED;
}