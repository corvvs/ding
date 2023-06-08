#include "ping.h"

timeval_t	get_current_time(void) {
	timeval_t	tv;
	gettimeofday(&tv, NULL);
	return tv;
}

void	normalize_time(timeval_t* t) {
	if (t->tv_usec < 0) {
		t->tv_sec += (t->tv_usec + 1) / 1000000 - 1;
		t->tv_usec -= ((t->tv_usec + 1) / 1000000 - 1) * 1000000;
	}
	if (t->tv_usec >= 1000000) {
		t->tv_sec += t->tv_usec / 1000000;
		t->tv_usec -= t->tv_usec / 1000000 * 1000000;
	}
}

timeval_t	add_times(timeval_t* a, timeval_t* b) {
	timeval_t	c = {
		.tv_sec = a->tv_sec + b->tv_sec,
		.tv_usec = a->tv_usec + b->tv_usec,
	};
	normalize_time(&c);
	return c;
}

timeval_t	sub_times(timeval_t* a, timeval_t* b) {
	timeval_t	c = {
		.tv_sec = a->tv_sec - b->tv_sec,
		.tv_usec = a->tv_usec - b->tv_usec,
	};
	normalize_time(&c);
	return c;
}
