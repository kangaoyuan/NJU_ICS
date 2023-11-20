#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
    int                NO;
    struct watchpoint* next;
    /* TODO: Add more members if necessary */
    uint32_t val;
    char     str[32];
} WP;

void new_wp(char* arg); 
void free_wp(int index);
bool check_wp();
void wp_display();
#endif
