#include <am.h>
#include <nemu.h>

static uint32_t st_time_lo, st_time_hi;

void __am_timer_init() {
    st_time_lo = inl(RTC_ADDR);
    st_time_hi = inl(RTC_ADDR+4);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    uint32_t now_time_lo = inl(RTC_ADDR);
    uint32_t now_time_hi = inl(RTC_ADDR+4);
    uptime->us = (now_time_lo - st_time_lo) + (now_time_hi - st_time_hi) * 1000000;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}
