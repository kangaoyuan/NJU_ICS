#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
    char               expr[256];
    int                NO, hit_num;
    uint32_t           pre_val, cur_val;
    struct watchpoint* next;

    /* TODO: Add more members if necessary */

} WP;

WP* new_wp();
void free_wp(uint32_t num);
void wp_pool_display();

#endif
