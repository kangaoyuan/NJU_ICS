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

    /* head insertion
    alloc->next = head;
    alloc->hit_num = 0;
    alloc->NO = ++WP_ID;
    head = alloc;
    */

    /* tail insertion */
    if (!head) {
        head = alloc;
        alloc->next = NULL;
        alloc->hit_num = 0;
        alloc->NO = ++WP_ID;
    } else {
        WP* cur = head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = alloc;
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

    if(!head)
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
    bool flag = false, result = false;
    for(WP* cur = head; cur; cur = cur->next){
        cur->pre_val = cur->cur_val;
        cur->cur_val = expr(cur->expr, &flag);
        Assert(flag == true, "check_wp expr failed");
        if(cur->pre_val != cur->cur_val){
            ++cur->hit_num;
            printf("Hardware watchpoint %d: %s\n\n", cur->NO,
                   cur->expr);
            printf("Old value = 0x%08x\t%08u\n", cur->pre_val, cur->pre_val);
            printf("New value = 0x%08x\t%08u\n", cur->cur_val, cur->cur_val);
            result = true;
        }
    }
    return result;
}

void wp_pool_display(){
    WP* cur = head;

    if(!cur)
        printf("None WatchPoints now\n");
    while(cur){
        printf(
            "WatchPoint %d already hit %d times\n%s == %08u\t0x%08x\n",
            cur->NO, cur->hit_num, cur->expr, cur->cur_val, cur->cur_val);
        cur = cur->next;
    }
}
