#include "expr.h"
#include "watchpoint.h"

#define NR_WP 32
static WP  wp_pool[NR_WP] = {};
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
void new_wp(char* arg) {
    if (free_ == NULL)
        panic("No more wp_pool memory space.");
    // get a now node for new action from head of free_ list.
    WP* now = free_;
    free_ = free_->next;
    now->next = NULL;

    strcpy(now->str, arg);
    bool     success;
    uint32_t init = expr(now->str, &success);
    if (success) {
        now->val = init;
        Log("Watchpoint %d: %s\nInitial value %u\n", now->NO, now->str,
            now->val);
    } else
        panic("watchpoint expr() eval failure.");

    // Attention 
    if (head == NULL)
        head = now;
    else {
        now->next = head->next;
        head->next = now;
    }
    return;
}

void free_wp(int index) {
    if (index < 0 || head == NULL)
        assert(0);

    int num = 0;
    WP* now = head;
    WP* target = NULL;
    while (now) {
        if (now->NO == index) {
            target = now;
            break;
        }
        if (now->next != NULL) {
            num++;
            now = now->next;
        }
    }
    if (target == NULL) {
        Log("Watchpoint %d not found!\n", index);
        return;
    }

    WP* before;
    WP* after = target->next;
    if (num == 0) {
        head = after;
    } else {
        now = head;
        while (num > 1) {
            now = now->next;
            num--;
        }
        before = now;
        before->next = after;
    }

    // Insert target node into the head of free_ list.
    target->next = free_;
    free_ = target;
    free_->val = 0;
    free_->str[0] = '\0';
    return;
}

void wp_display() {
    WP* now = head;
    if (now == NULL)
        printf("No watchpoints.\n");
    while (now) {
        printf("Watchpoint %d: %s = %u\n", now->NO, now->str, now->val);
        now = now->next;
    }
    return;
}

bool check_wp(){
    bool flag = false;
    for(WP *now = head; now; now = now->next){
        bool success;
        uint32_t val = now->val; 
        uint32_t real = expr(now->str, &success);
        if (success) {
            if (val != real) {
                printf(
                    "Watchpoint %d: %s\nOld value = %u\nNew value = %u\n",
                    now->NO, now->str, val, real);
                now->val = real;
                flag = true;
            }
        } else
            panic("watchpoint expr() eval failure.");
    }
    return flag;
}
