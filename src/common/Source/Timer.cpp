#include "common/Timer.h"

void Timer_GetSystemTime(TimerInfo* pTimerInfo) {
	OSCalendarTime calendarTime;

    OSTicksToCalendarTime(OSGetTime(), &calendarTime);
    pTimerInfo->hours = calendarTime.hour;
    pTimerInfo->minutes = calendarTime.min;
    pTimerInfo->seconds = calendarTime.sec;
    pTimerInfo->milliseconds = calendarTime.msec;
    pTimerInfo->day = calendarTime.mday;
    pTimerInfo->month = calendarTime.mon;
    pTimerInfo->year = calendarTime.year;
};

int Timer_GetDHMSInSeconds(TimerInfo *pTimerInfo) {
    int ret = pTimerInfo->day * 0x15180;
    ret += pTimerInfo->hours * 0xE10;
    ret += pTimerInfo->minutes * 0x3C;
    ret += pTimerInfo->seconds;
    return ret;
}

inline void Timer_SetDHMSInSeconds(struct TimerInfo *pTimerInfo, int time) {
    pTimerInfo->seconds = time % 60;
    time /= 60;
    pTimerInfo->minutes = time % 60;
    time /= 60;
    pTimerInfo->hours = time % 24;
    pTimerInfo->day =  time / 24;
}

void Timer_GetDifference(TimerInfo *pTimeDiff, TimerInfo *param_2, TimerInfo *param_3) {
	int time = Timer_GetDHMSInSeconds(param_2);
	int time2 = Timer_GetDHMSInSeconds(param_3);
    
    pTimeDiff->milliseconds = 0;
    pTimeDiff->month = 0;
    pTimeDiff->year = 0;
    Timer_SetDHMSInSeconds(pTimeDiff, time2 - time);
    return;
}