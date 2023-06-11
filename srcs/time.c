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

timeval_t	add_times(const timeval_t* a, const timeval_t* b) {
	timeval_t	c = {
		.tv_sec = a->tv_sec + b->tv_sec,
		.tv_usec = a->tv_usec + b->tv_usec,
	};
	normalize_time(&c);
	return c;
}

timeval_t	sub_times(const timeval_t* a, const timeval_t* b) {
	timeval_t	c = {
		.tv_sec = a->tv_sec - b->tv_sec,
		.tv_usec = a->tv_usec - b->tv_usec,
	};
	normalize_time(&c);
	return c;
}

double	get_ms(const timeval_t* a) {
	return a->tv_sec * 1000.0 + a->tv_usec / 1000.0;
}
