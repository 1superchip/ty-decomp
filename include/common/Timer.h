#include "types.h"
#include "Dolphin/os/OSTime.h"

extern "C" void OSTicksToCalendarTime(OSTime ticks, OSCalendarTime* td);
extern "C" OSTime OSGetTime(void);


struct TimerInfo {
	s32 hours;
	s32 minutes;
	s32 seconds;
	s32 milliseconds;
	s32 day;
	s32 month;
	s32 year;
};

void Timer_GetSystemTime(TimerInfo*);
s32 Timer_GetDHMSInSeconds(TimerInfo*);
void Timer_GetDifference(TimerInfo*, TimerInfo*, TimerInfo*);