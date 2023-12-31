#include <am.h>
#include <x86.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

Context* __am_irq_handle(Context* c) {
    /*printf("eax:%08x\necx:%08x\nedx:%08x\nebx:%08x\nesp:%08x\nebp:%"
           "08x\nesi:%08x\nedi:%08x\n",
           c->eax, c->ecx, c->edx, c->ebx, c->esp, c->ebp, c->esi, c->edi);
    printf("cr3:%p\n", c->cr3);
    printf("irq:%d\n", c->irq);
    printf("pc:%08x\ncs:%08x\neflags:%08x\n", c->eip, c->cs, c->eflags);*/
    if (user_handler) {
        Event ev = {0};
        switch (c->irq) {
        case 0x80:
            ev.event = EVENT_SYSCALL;
            break;
        case 0x81:
            ev.event = EVENT_YIELD;
            break;
        case 0x32:
            ev.event = EVENT_IRQ_TIMER;
            break;
        default:
            ev.event = EVENT_ERROR;
            break;
        }
        // user_handler is register as do_event() in init_irq() -> cte_init().
        c = user_handler(ev, c);
        assert(c != NULL);
    }
    return c;
}

bool cte_init(Context* (*handler)(Event, Context*)) {
    static GateDesc32 idt[NR_IRQ];

    // initialize IDT, for int instruction executing.
    for (unsigned int i = 0; i < NR_IRQ; i++) 
        idt[i] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);

    // IRQ_TIMER
    idt[32] = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0, DPL_KERN);
    // SYSCALL
    idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys, DPL_USER);
    // YIELD
    idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);
    set_idt(idt, sizeof(idt));

    // register interrupt event handler, for above __am_irq_handle to run.
    user_handler = handler;
    return true;
}

Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
    asm volatile("int $0x81");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
