#include "ping.h"
#define EPS 1e-6

double	ft_fabs(double x) {
	if (x < 0) {
		return -x;
	}
	return x;
}

double	ft_square(double x) {
	return x * x;
}

double	ft_sqrt(double x) {
	if (x <= 0) {
		return 0;
	}
	double ymin = 0;
	double ymax = x;
	for (size_t i = 0; i < 1024; ++i) {
		double y = ymin / 2 + ymax / 2;
		double z = y * y;
		double diff = ft_fabs(x - z);
		if (diff < EPS) {
			break;
		}
		if (x < z) {
			ymax = y;
		} else if (x > z) {
			ymin = y;
		}
	}
	double rv = ymin / 2 + ymax / 2;
	DEBUGOUT("%f -> %f -> %f", x, rv, rv * rv);
	return rv;
}
