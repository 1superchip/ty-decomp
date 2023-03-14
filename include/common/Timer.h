#include "types.h"
#include "Dolphin/os/OSTime.h"

extern "C" void OSTicksToCalendarTime(OSTime ticks, OSCalendarTime* td);
extern "C" OSTime OSGetTime(void);


struct TimerInfo {
	int hours;
	int minutes;
	int seconds;
	int milliseconds;
	int day;
	int month;
	int year;
};

void Timer_GetSystemTime(TimerInfo*);
int Timer_GetDHMSInSeconds(TimerInfo*);
void Timer_GetDifference(TimerInfo*, TimerInfo*, TimerInfo*);