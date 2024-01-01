#include <NDL.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>

#define HSA_NDL
#ifndef HAS_NDL

int main() {
    size_t cnt = 0;
    struct timeval init, now;

    assert(gettimeofday(&init, NULL) == 0);
    time_t init_sec = init.tv_sec;
    suseconds_t init_usec = init.tv_usec;
    
    while(true){
        assert(gettimeofday(&now, NULL) == 0);
        time_t now_sec = now.tv_sec;
        suseconds_t now_usec = now.tv_usec;
        uint64_t time_gap = (now_sec - init_sec) * 1000000 + (now_usec - init_usec); 
        if(time_gap > 500000 * cnt)
            printf("Half a second flowed, %u time\n", cnt++); 
        printf("%d loop\n", cnt);
    } 

    return 0;
}

#else


int main() {
    NDL_Init(0);
    printf("NDL_GetTicks test start...\n");

    uint32_t init = NDL_GetTicks();
    size_t times = 1;

    while (1) {
        uint32_t now = NDL_GetTicks();
        uint32_t time_gap = now - init;
        if (time_gap > 500 * times) {
            printf("Half a second passed, %u time(s)\n", times);
            times++;
        }
    }
}

#endif
