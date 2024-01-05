#include <common.h>
#include <sys/time.h>
#include <signal.h>

#define TIMER_HZ 60
#define MAX_HANDLER 8

static uint32_t jiffy = 0;
uint32_t uptime() { return jiffy / TIMER_HZ; }

static int idx = 0;
typedef void (*alarm_handler_t) ();
static alarm_handler_t handler[MAX_HANDLER] = {};

void add_alarm_handle(void* h) {
    assert(idx < MAX_HANDLER);
    handler[idx++] = h;
}

static void alarm_sig_handler(int signum) {
    for (int i = 0; i < idx; i++) {
        handler[i]();
    }
    jiffy++;
}

void init_alarm() {
    /* The sigaction() system call is used to change the action taken by a
     * process on receipt of a specific signal. */
    // Two can't be signalaction processing signal are SIGKILL and SIGSTOP
    // struct sigaction's sa_handler field specifies the action to associated signum with SIG_DFL, SIG_IGN or A pointer to a signal handling function.
    // sa_mask field in struct sigaction specifies a mask of signals which should be blocked during execution of the sa_hangdler. Additionaly, the signal which triggered the handler self will be blocked.
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    s.sa_handler = alarm_sig_handler;
    int ret = sigaction(SIGVTALRM, &s, NULL);
    Assert(ret == 0, "Can not set signal handler");

    struct itimerval it = {};
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000000 / TIMER_HZ;
    it.it_interval = it.it_value;
    // ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF
    ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
    Assert(ret == 0, "Can not set timer");
}
