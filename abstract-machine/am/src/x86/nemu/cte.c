#include <am.h>
#include <x86/x86.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2

// Context is defined inside x86-nemu.h for specific arch
static Context* (*user_handler)(Event, Context*) = NULL;

// Defined inside trap.S called through IDT.
void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

// called from above exception/interpret entry point to do the work registered from OS.
Context* __am_irq_handle(Context* c) {
    printf("inside __am_irq_handle: address of c 0x%x\n", c);
    printf("inside __am_irq_handle: irq of c %d\n", c->irq);
    if (user_handler) {
        Event ev = {0};
        // Construct event to the user_handler registered by OS from Context.
        switch (c->irq) {
        case 32:
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

        c = user_handler(ev, c);
        assert(c != NULL);
    }

    return c;
}

// handler is from the OS to callback when the exception/interpret event happens.
bool cte_init(Context* (*handler)(Event, Context*)) {
    // register event(exception/interpret) handler
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

    printf("Before set_idt, I can tell your the address of idt  and address of idt[0x81]: %p, %x\n", idt, (idt[0x81].off_31_16 << 16) + idt[0x81].off_15_0);
    set_idt(idt, sizeof(idt));

    return true;
}

Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
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
}
