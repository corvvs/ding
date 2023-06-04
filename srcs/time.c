#include "ping.h"

timeval_t	get_current_time(void) {
	timeval_t	tv;
	gettimeofday(&tv, NULL);
	return tv;
}

double		get_current_epoch_ms(void) {
	timeval_t	tv = get_current_time();
	return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}
