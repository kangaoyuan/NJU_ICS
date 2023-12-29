#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>

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
        uint64_t time_gap = (now_sec - init_sec) * 10e6 + (now_usec - init_usec); 
        if(time_gap > 5 * 10e5)
            printf("Half a second flowed, %u time\n", cnt++); 
    } 

    return 0;
}
