#include <common.h>

void do_syscall(Context* c);
Context* schedule(Context* prev);

static Context* do_event(Event e, Context* c) {
    switch (e.event) {
    case EVENT_SYSCALL:
        do_syscall(c);
        break;
    case EVENT_YIELD:
        schedule(c);
        break;
    case EVENT_IRQ_TIMER:
        schedule(c);
        break; 
    case EVENT_IRQ_IODEV:
        Log("EVENT_IRQ_IODEV recognized in do_event(Event, Context)");
        break; 
    default:
        panic("Unhandled event ID = %d", e.event);
    }

    // do_event() as a user __am_irq_handle() callback, returan value go to the calling point in __am_irq_handle() 
    return c;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    cte_init(do_event);
}
