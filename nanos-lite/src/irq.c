#include <common.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
    static int timer_cnt = 0;
    switch (e.event) {
    case EVENT_SYSCALL:
        do_syscall(c);
        break;
    case EVENT_YIELD:
        printf("In nanos, Goal Yield\n");
        c = schedule(c);
        break;
    case EVENT_IRQ_TIMER:
        timer_cnt++;
        if(timer_cnt % 0x64 == 0){
            timer_cnt = 0;
            printf("In nanos, Got Timer Irq\n");
        }
        c = schedule(c);
        break;
    default:
        panic("Unhandled event ID = %d", e.event);
    }

    return c;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    cte_init(do_event);
}
