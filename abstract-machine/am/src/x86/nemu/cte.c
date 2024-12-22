#include <am.h>
#include <x86/x86.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2

// Context is defined inside x86-nemu.h for specific arch
static Context* (*user_handler)(Event, Context*) = NULL;

// Defined inside trap.S, called through IDT.
void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

// called from above exception/interpret routine(int instruction entry point) 
// to do the work registered from OS by user_handler function.
Context* __am_irq_handle(Context* c) {
    if (user_handler) {
        Event ev = {0};
        // Construct event to the user_handler registered by OS from Context.
        switch (c->irq) {
        case 0x20:
            ev.event = EVENT_IRQ_TIMER;
            break;
        case 0x80:
            ev.event = EVENT_SYSCALL;
            break;
        case 0x81:
            ev.event = EVENT_YIELD;
            break;
        default:
            ev.event = EVENT_ERROR;
            break;
        }

        // registered by os.
        c = user_handler(ev, c);
        assert(c != NULL);
    }

    return c;
}

// handler is from the OS to callback when the exception/interpret event happens.
bool cte_init(Context* (*handler)(Event, Context*)) {
    // register event(exception/interpret) handler written by OS.
    user_handler = handler;

    // initialize IDT(exception/interpret entry point array)
    static GateDesc32 idt[NR_IRQ];

    for (unsigned int i = 0; i < NR_IRQ; i++) {
        idt[i] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);
    }

    // ----------------------- interrupts ----------------------------
    idt[32] = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0, DPL_KERN);
    // ---------------------- system call ----------------------------
    idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys, DPL_USER);
    idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

    set_idt(idt, sizeof(idt));

    return true;
}

// Create a context at the end of kernel stack.
Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
    Context* kcontext = kstack.end - sizeof(Context) - 2*sizeof(uintptr_t);
    *(uintptr_t*)(kstack.end - sizeof(uintptr_t)) = (uintptr_t)arg;
    *kcontext = (Context){.cr3 = NULL,
                 .eip = (uintptr_t)entry,
                 .cs  = 0x8,
                 .eflags = 0x200};
    return kcontext;
}

void yield() {
    // EVENT_YIELD
    printf("In AM, Got yield\n");
    asm volatile("int $0x81");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {
    if(enable)
        sti();
    else
        cli();
}
