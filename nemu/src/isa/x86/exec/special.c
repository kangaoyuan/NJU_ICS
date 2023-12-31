#include <cpu/exec.h>
#include <monitor/monitor.h>
#include <monitor/difftest.h>
#include <monitor/log.h>

def_EHelper(nop) {
    print_asm("nop");
    return;
}

def_EHelper(nemu_trap) {
    difftest_skip_ref();

    rtl_exit(NEMU_END, cpu.pc, cpu.eax);

    print_asm("nemu trap");
    return;
}

/* invalid opcode */
def_EHelper(inv) {
    uint32_t temp[2];
    char tempbuf[256] = {};
    extern char log_bytebuf[80];

    snprintf(tempbuf, sizeof(tempbuf), FMT_WORD ":   %s%*.s%s", cpu.pc,
             log_bytebuf, 50 - (12 + 3 * (s->seq_pc - cpu.pc)), "",
             "error occured");
    puts(tempbuf);
    log_write("%s\n", tempbuf);

    s->seq_pc = cpu.pc;
    log_bytebuf[0] = '\0';
    temp[0] = instr_fetch(&s->seq_pc, 4);
    temp[1] = instr_fetch(&s->seq_pc, 4);

    uint8_t* p = (uint8_t*)(void*)temp;
    printf("invalid opcode(PC = 0x%08x): %02x %02x %02x %02x %02x %02x "
           "%02x %02x ...\n\n",
           cpu.pc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

    display_inv_msg(cpu.pc);

    rtl_exit(NEMU_ABORT, cpu.pc, -1);

    print_asm("invalid opcode");
    return;
}
