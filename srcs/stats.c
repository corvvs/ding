#include "ping.h"

static void	extend_buffer(t_stat_data* stat_data) {
	stat_data->rtts_cap = stat_data->rtts_cap > 0 ? stat_data->rtts_cap * 2 : 32;
	double*	extended = malloc(sizeof(double) * stat_data->rtts_cap);
	ft_memcpy(extended, stat_data->rtts, sizeof(double) * stat_data->packets_received);
	free(stat_data->rtts);
	stat_data->rtts = extended;
}

// ASSERTION: received_icmp のサイズが sizeof(timeval_t) 以上
double	mark_received(t_ping* ping, const t_acceptance* acceptance) {
	// アライメント違反を防ぐために, 一旦別バッファにコピーする.
	uint8_t			buffer_sent[sizeof(timeval_t)];
	const uint8_t*	received_icmp = acceptance->recv_buffer + sizeof(ip_header_t);
	ft_memcpy(buffer_sent, received_icmp + sizeof(icmp_header_t), sizeof(timeval_t));

	t_stat_data*		stat_data = &ping->target.stat_data;
	const timeval_t*	epoch_sent = (const timeval_t*)buffer_sent;
	if (stat_data->packets_received >= stat_data->rtts_cap) {
		extend_buffer(stat_data);
	}

	double rtt = diff_times(&acceptance->epoch_received, epoch_sent);
	stat_data->rtts[stat_data->packets_received] = rtt;
	stat_data->packets_received += 1;
	return rtt;
}

static double	rtt_min(const t_stat_data* stat_data) {
	if (stat_data->packets_received == 0) {
		return 0;
	}
	double	min = stat_data->rtts[0];
	for (size_t i = 0; i < stat_data->packets_received; i += 1) {
		if (stat_data->rtts[i] < min) {
			min = stat_data->rtts[i];
		}
	}
	return min;
}

static double	rtt_max(const t_stat_data* stat_data) {
	if (stat_data->packets_received == 0) {
		return 0;
	}
	double	max = stat_data->rtts[0];
	for (size_t i = 0; i < stat_data->packets_received; i += 1) {
		if (stat_data->rtts[i] > max) {
			max = stat_data->rtts[i];
		}
	}
	return max;
}

static double	rtt_average(const t_stat_data* stat_data) {
	if (stat_data->packets_received == 0) {
		return 0;
	}
	double	sum = 0;
	for (size_t i = 0; i < stat_data->packets_received; i += 1) {
		sum += stat_data->rtts[i];
	}
	return sum / stat_data->packets_received;
}

static double	rtt_stddev(const t_stat_data* stat_data) {
	if (stat_data->packets_received == 0) {
		return 0;
	}
	double	average = rtt_average(stat_data);
	double	sum = 0;
	for (size_t i = 0; i < stat_data->packets_received; i += 1) {
		sum += ft_square(stat_data->rtts[i]);
	}
	return ft_sqrt(sum / stat_data->packets_received - average * average);
}

// 統計データのリボン
static void	print_stats_ribbon(const t_ping* ping) {
	printf("--- %s ping statistics ---\n", ping->target.given_host);
}

// パケットロスに関する統計データを表示
static void	print_stats_packet_loss(const t_stat_data* stat_data) {
	printf("%zu packets transmitted, ", stat_data->packets_sent);
	printf("%zu packets received, ", stat_data->packets_received);
	if (stat_data->packets_sent > 0) {
		double gain_rate = (double)stat_data->packets_received / stat_data->packets_sent;
		printf("%d%% packet loss", (int)((1 - gain_rate) * 100));
	}
	printf("\n");
}

// ラウンドトリップに関する統計データを表示
static void	print_stats_roundtrip(const t_stat_data* stat_data) {
	// 受信パケットがない場合は統計データを出さない
	if (stat_data->packets_received == 0) { return; }
	double	min = rtt_min(stat_data);
	double	max = rtt_max(stat_data);
	double	avg = rtt_average(stat_data);
	double	stddev = rtt_stddev(stat_data);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", min, avg, max, stddev);
}

void	print_stats(const t_ping* ping) {
	print_stats_ribbon(ping);
	print_stats_packet_loss(&ping->target.stat_data);
	print_stats_roundtrip(&ping->target.stat_data);
}
