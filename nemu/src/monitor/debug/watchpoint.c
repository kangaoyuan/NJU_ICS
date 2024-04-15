#include "expr.h"
#include "watchpoint.h"

#define NR_WP 32

static uint32_t WP_ID;
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    for (int i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    WP_ID = 0;
    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
    if(!free_)
        panic("no allocation space to new_wp");

    WP* alloc = free_;
    free_ = free_->next;

    if (!head) {
        head = alloc;
        alloc->next = NULL;
        alloc->hit_num = 0;
        alloc->NO = ++WP_ID;
    } else {
        /* tail insertion */
        WP* cur = head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = alloc;
        /* head insertion */
        //head->next = alloc;
        alloc->next = NULL;
        alloc->hit_num = 0;
        alloc->NO = ++WP_ID;
    }
    return alloc;
}
void free_wp(uint32_t num){
    WP* cur = head;

    if(cur && cur->NO == num){
        head = head->next;
        cur->next = free_;
        free_ = cur;
        return;
    }

    if(!cur)
        return;

    while(cur->next){
        if(cur->next->NO == num){
            break;
        }
        cur = cur->next;
    }

    if(!cur->next)
        return;

    cur->next = cur->next->next;
    cur->next->next = free_;
    free_ = cur->next;
};

bool check_wp(){
    bool flag;
    for(WP* cur = head; cur; cur = cur->next){
        cur->pre_val = cur->cur_val;
        cur->cur_val = expr(cur->expr, &flag);
        if(flag && cur->cur_val != cur->pre_val){
            ++cur->hit_num;
            return true;
        }
    }
    return false;
}

void wp_pool_display(){
    WP* cur = head;

    while(cur){
        bool flag;
        uint32_t res = expr(cur->expr, &flag);
        if(!flag)
            panic("watchpoint expr() failed");
        printf("WatchPoint %d:\t%s = %u\n", cur->NO, cur->expr, res);
        cur = cur->next;
    }
}
