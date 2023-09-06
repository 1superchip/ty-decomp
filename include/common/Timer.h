#ifndef COMMON_TIMER
#define COMMON_TIMER

#include "types.h"

#define MINUTES_TO_SECONDS 60
#define HOURS_TO_SECONDS 3600
#define DAYS_TO_SECONDS 86400
#define MONTH_TO_SECONDS (30 * DAYS_TO_SECONDS)

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

#endif // COMMON_TIMER
