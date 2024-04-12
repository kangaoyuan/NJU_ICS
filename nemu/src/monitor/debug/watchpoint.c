#include "expr.h"
#include "watchpoint.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    for (int i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
    WP* alloc = free_;
    free_ = free_->next;

    if (!head) {
        head = alloc;
        head->next = NULL;
    } else {
        while (head->next) {
            head = head->next;
        }
        head->next = alloc;
        alloc->next = NULL;
    }
    return alloc;
}
void free_wp(WP *wp){
    wp->next = free_;
    free_ = wp;
};

void wp_pool_display(){
    while(head){
        bool flag;
        uint32_t res = expr(head->expr, &flag);
        if(!flag)
            panic("watchpoint expr() failed");
        printf("%d:\t%s:\t%u\n", head->NO, head->expr, res);
        head = head->next;
    }
}
