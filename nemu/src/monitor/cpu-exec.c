#include <isa.h>
#include <monitor/log.h>
#include <monitor/monitor.h>
#include <monitor/difftest.h>
#include <stdlib.h>
#include <sys/time.h>

// control when the log is printed, unit: number of instructions
#define LOG_START (0)
// restrict the size of log file
#define LOG_END   (1024 * 1024 * 50)
// The assembly code of instructions executed is only output to the screen
#define MAX_INSTR_TO_PRINT 10
#pragma GCC diagnostic ignored "-Wunused-const-variable"

CPU_state cpu = {};
const rtlreg_t rzero = 0;
NEMUState nemu_state = { .state = NEMU_STOP };
static uint64_t g_timer = 0; // unit: ms
static uint64_t g_nr_guest_instr = 0;

bool check_wp();
void asm_print(vaddr_t this_pc, int instr_len, bool print_flag);


bool log_enable() {
    return (g_nr_guest_instr >= LOG_START) && (g_nr_guest_instr <= LOG_END);
}

void rtl_exit(int state, vaddr_t halt_pc, uint32_t halt_ret) {
    nemu_state = (NEMUState){
        .state = state, .halt_pc = halt_pc, .halt_ret = halt_ret};
}

int is_exit_status_bad() {
    int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) ||
               (nemu_state.state == NEMU_QUIT);
    return !good;
}

void monitor_statistic() {
    Log("total guest instructions = %lu", g_nr_guest_instr);
    Log("host time spent = %lu ms", g_timer);
    if (g_timer > 0)
        Log("simulation frequency = %lu instr/s",
            g_nr_guest_instr * 1000 / g_timer);
    else
        Log("Finish running in less than 1 ms and can not calculate the "
            "simulation frequency");
}

void display_inv_msg(vaddr_t pc) {
    printf("There are two cases, which will trigger this unexpected "
           "exception:\n"
           "1. The instruction at PC = " FMT_WORD " is not implemented.\n"
           "2. Something is implemented incorrectly.\n",
           pc);
    printf("Find this PC(" FMT_WORD ") in the disassembling result to "
           "distinguish which case it is.\n\n",
           pc);
    printf("\33[1;31mIf it is the first case, see\n%s\nfor more "
           "details.\n\nIf it is the second case, remember:\n"
           "* The machine is always right!\n"
           "* Every line of untested code is always wrong!\33[0m\n\n",
           isa_logo);
}

static uint64_t get_time() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint32_t seconds = now.tv_sec;
    uint32_t useconds = now.tv_usec;
    return seconds * 1000 + (useconds + 500) / 1000;
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
    switch (nemu_state.state) {
    case NEMU_END:
    case NEMU_ABORT:
        printf("Program execution has ended. To restart the program, exit "
               "NEMU and run again.\n");
        return;
    default:
        nemu_state.state = NEMU_RUNNING;
    }

    uint64_t timer_start = get_time();

    for (; n > 0; n--) {
        __attribute__((unused)) vaddr_t this_pc = cpu.pc;

        /* Execute one instruction, including instruction fetch,
         * instruction decode, and the actual execution. */
        __attribute__((unused)) vaddr_t seq_pc = isa_exec_once();
        g_nr_guest_instr++;

#ifdef DIFF_TEST
        difftest_step(this_pc, cpu.pc);
#endif

#ifdef DEBUG
        asm_print(this_pc, seq_pc - this_pc, n < MAX_INSTR_TO_PRINT);

        /* TODO: check watchpoints here. */
        if (check_wp() && nemu_state.state != NEMU_ABORT &&
            nemu_state.state != NEMU_END)
            nemu_state.state = NEMU_STOP;
#endif

#ifdef HAS_IOE
        void device_update();
        device_update();
#endif

        if (nemu_state.state != NEMU_RUNNING)
            break;
    }

    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (nemu_state.state) {
    case NEMU_RUNNING:
        nemu_state.state = NEMU_STOP;
        break;
    case NEMU_END:
    case NEMU_ABORT:
        Log("nemu: %s\33[0m at pc = " FMT_WORD "\n",
            (nemu_state.state == NEMU_ABORT
                 ? "\33[1;31mABORT"
                 : (nemu_state.halt_ret == 0 ? "\33[1;32mHIT GOOD TRAP"
                                             : "\33[1;31mHIT BAD TRAP")),
            nemu_state.halt_pc);
        // fall through
    case NEMU_QUIT:
        monitor_statistic();
    }
}
