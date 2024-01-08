#include <am.h>
#include <nemu.h>

#define RTC_ADDR_SEC (RTC_ADDR + 4)
static uint64_t st_time = 0;

void __am_timer_init() {
    st_time = inl(RTC_ADDR_SEC) * 1000000 + inl(RTC_ADDR) + 500;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T* uptime) {
    uint64_t ed_time = inl(RTC_ADDR_SEC) * 1000000 + inl(RTC_ADDR) + 500;
    uptime->us = ed_time - st_time; 
}

void __am_timer_rtc(AM_TIMER_RTC_T* rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}
