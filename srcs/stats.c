#include "ping.h"

static void	extend_buffer(t_stat_data* stat_data) {
	stat_data->rtts_cap = stat_data->rtts_cap > 0 ? stat_data->rtts_cap * 2 : 32;
	double*	extended = malloc(sizeof(double) * stat_data->rtts_cap);
	ft_memcpy(extended, stat_data->rtts, sizeof(double) * stat_data->packets_receipt);
	free(stat_data->rtts);
	stat_data->rtts = extended;
}

timeval_t	mark_sent(t_ping* ping) {
	ping->stat_data.packets_sent += 1;
	return get_current_time();
}

double	mark_receipt(t_ping* ping, const timeval_t* epoch_sent) {
	if (ping->stat_data.packets_receipt >= ping->stat_data.rtts_cap) {
		extend_buffer(&ping->stat_data);
	}
	timeval_t epoch_receipt = get_current_time();
	double rtt =
		(epoch_receipt.tv_sec - epoch_sent->tv_sec) * 1000.0 +
		(epoch_receipt.tv_usec - epoch_sent->tv_usec) / 1000.0;
	ping->stat_data.rtts[ping->stat_data.packets_receipt] = rtt;
	ping->stat_data.packets_receipt += 1;
	return rtt;
}

static double	rtt_min(const t_ping* ping) {
	if (ping->stat_data.packets_receipt == 0) {
		return 0;
	}
	double	min = ping->stat_data.rtts[0];
	for (size_t i = 0; i < ping->stat_data.packets_receipt; i += 1) {
		if (ping->stat_data.rtts[i] < min) {
			min = ping->stat_data.rtts[i];
		}
	}
	return min;
}

static double	rtt_max(const t_ping* ping) {
	if (ping->stat_data.packets_receipt == 0) {
		return 0;
	}
	double	max = ping->stat_data.rtts[0];
	for (size_t i = 0; i < ping->stat_data.packets_receipt; i += 1) {
		if (ping->stat_data.rtts[i] > max) {
			max = ping->stat_data.rtts[i];
		}
	}
	return max;
}

static double	rtt_average(const t_ping* ping) {
	if (ping->stat_data.packets_receipt == 0) {
		return 0;
	}
	double	sum = 0;
	for (size_t i = 0; i < ping->stat_data.packets_receipt; i += 1) {
		sum += ping->stat_data.rtts[i];
	}
	return sum / ping->stat_data.packets_receipt;
}

static double	rtt_stddev(const t_ping* ping) {
	if (ping->stat_data.packets_receipt == 0) {
		return 0;
	}
	double	average = rtt_average(ping);
	double	sum = 0;
	for (size_t i = 0; i < ping->stat_data.packets_receipt; i += 1) {
		sum += ft_square(ping->stat_data.rtts[i]);
	}
	return ft_sqrt(sum / ping->stat_data.packets_receipt - average * average);
}

// 統計データのリボン
static void	print_stats_ribbon(const t_ping* ping) {
	printf("--- %s ping statistics ---\n", ping->target.given_host);
}

// パケットロスに関する統計データを表示
static void	print_stats_packet_loss(const t_ping* ping) {
	printf("%zu packets transmitted, %zu packets received, %d%% packet loss\n",
		ping->stat_data.packets_sent,
		ping->stat_data.packets_receipt,
		(int)((1 - (double)ping->stat_data.packets_receipt / ping->stat_data.packets_sent) * 100)
	);
}

// ラウンドトリップに関する統計データを表示
static void	print_stats_roundtrip(const t_ping* ping) {
	// 受信パケットがない場合は統計データを出さない
	if (ping->stat_data.packets_receipt == 0) { return; }
	double	min = rtt_min(ping);
	double	max = rtt_max(ping);
	double	avg = rtt_average(ping);
	double	stddev = rtt_stddev(ping);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", min, avg, max, stddev);
}

void	print_stats(const t_ping* ping) {
	print_stats_ribbon(ping);
	print_stats_packet_loss(ping);
	print_stats_roundtrip(ping);
}
