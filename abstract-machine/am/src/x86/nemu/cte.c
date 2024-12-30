#include <am.h>
#include <x86/x86.h>
#include <klib.h>

#define NR_SEG         6
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

void __am_switch(Context *c);
void __am_get_cur_as(Context *c);

// called from above exception/interpret routine(int instruction entry point) 
// to do the work registered from OS by user_handler function, and then return the context.
Context* __am_irq_handle(Context* c) {
    // Save current as to the context
    __am_get_cur_as(c);

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

        // registered by OS.
        c = user_handler(ev, c);
        assert(c != NULL);
    }

    // Restore the as from the context
    __am_switch(c);
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

    // initialize TSS
    static TSS32 tss = {};
    tss.ss0 = KSEL(2);
    // ltr to set cpu.TR
    set_tr(KSEL(5));

    // initialize GDT
    static SegDesc gdt[NR_SEG] = {};
    gdt[1] = SEG32(STA_X | STA_R, 0, 0xffffffff, DPL_KERN);
    gdt[2] = SEG32(STA_W, 0, 0xffffffff, DPL_KERN);
    gdt[3] = SEG32(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
    gdt[4] = SEG32(STA_W, 0, 0xffffffff, DPL_USER);
    gdt[5] = SEG16(STS_T32A, &tss, sizeof(tss) - 1, DPL_KERN);
    // lgdt to set cpu.gdtr content
    set_gdt(gdt, sizeof(gdt[0]) * NR_SEG);

    return true;
}

// Create a context at the end of kernel stack.
Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
    Context* kcontext = kstack.end - sizeof(Context) - 2*sizeof(uintptr_t);
    *(uintptr_t*)(kstack.end - sizeof(uintptr_t)) = (uintptr_t)arg;
    *kcontext = (Context){
        .cr3 = NULL,
        .eip = (uintptr_t)entry,
        .cs  = KSEL(1),
        .eflags = 0x200
    };
    return kcontext;
}

void yield() {
    // EVENT_YIELD
    //printf("In AM, Got yield\n");
    asm volatile("int $0x81");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {
    (void)enable;
}
