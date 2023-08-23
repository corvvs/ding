#include "ping.h"

static void	extend_roundtrip_times(t_stat_data* stat_data) {
	stat_data->roundtrip_times_cap = stat_data->roundtrip_times_cap > 0 ? stat_data->roundtrip_times_cap * 2 : 32;
	double*	extended = malloc(sizeof(double) * stat_data->roundtrip_times_cap);
	ft_memcpy(extended, stat_data->roundtrip_times, sizeof(double) * stat_data->received_echo_replies_with_ts);
	free(stat_data->roundtrip_times);
	stat_data->roundtrip_times = extended;
}

static double	record_roundtrip_time(t_stat_data* stat_data, const t_acceptance* acceptance) {
	// NOTE: メモリアライメント違反を防ぐため, 一旦別バッファにコピーしてからアクセスする
	uint8_t				buffer_sent[sizeof(timeval_t)];
	const ip_header_t*	ip_header = (const ip_header_t*)acceptance->recv_buffer;
	const size_t		ip_header_len = ihl_to_octets(ip_header->IP_HEADER_HL);
	const uint8_t*		received_icmp = acceptance->recv_buffer + ip_header_len;
	if (ip_header_len + sizeof(icmp_header_t) + sizeof(timeval_t) > acceptance->received_len) {
		DEBUGWARN("received_len is too short to carry timestamp: received_len: %zu", acceptance->received_len);
		return -1;
	}
	ft_memcpy(buffer_sent, received_icmp + sizeof(icmp_header_t), sizeof(timeval_t));

	const timeval_t*	epoch_sent = (const timeval_t*)buffer_sent;
	if (stat_data->received_echo_replies_with_ts >= stat_data->roundtrip_times_cap) {
		extend_roundtrip_times(stat_data);
	}

	double	rtt = diff_times(&acceptance->epoch_received, epoch_sent);
	stat_data->roundtrip_times[stat_data->received_echo_replies_with_ts] = rtt;
	return rtt;
}

static double	get_rtt_min(const t_stat_data* stat_data) {
	if (stat_data->received_echo_replies_with_ts == 0) {
		return 0;
	}
	double	min = stat_data->roundtrip_times[0];
	for (size_t i = 0; i < stat_data->received_echo_replies_with_ts; i += 1) {
		if (stat_data->roundtrip_times[i] < min) {
			min = stat_data->roundtrip_times[i];
		}
	}
	return min;
}

static double	get_rtt_max(const t_stat_data* stat_data) {
	if (stat_data->received_echo_replies_with_ts == 0) {
		return 0;
	}
	double	max = stat_data->roundtrip_times[0];
	for (size_t i = 0; i < stat_data->received_echo_replies_with_ts; i += 1) {
		if (stat_data->roundtrip_times[i] > max) {
			max = stat_data->roundtrip_times[i];
		}
	}
	return max;
}

static double	get_rtt_average(const t_stat_data* stat_data) {
	if (stat_data->received_echo_replies_with_ts == 0) {
		return 0;
	}
	double	sum = 0;
	for (size_t i = 0; i < stat_data->received_echo_replies_with_ts; i += 1) {
		sum += stat_data->roundtrip_times[i];
	}
	return sum / stat_data->received_echo_replies_with_ts;
}

static double	get_rtt_stddev(const t_stat_data* stat_data) {
	if (stat_data->received_echo_replies_with_ts == 0) {
		return 0;
	}
	double	average = get_rtt_average(stat_data);
	double	sum = 0;
	for (size_t i = 0; i < stat_data->received_echo_replies_with_ts; i += 1) {
		sum += ft_square(stat_data->roundtrip_times[i]);
	}
	return ft_sqrt(sum / stat_data->received_echo_replies_with_ts - average * average);
}

// 統計データのリボン
static void	print_stats_ribbon(const t_ping* ping) {
	printf("--- %s ping statistics ---\n", ping->target.given_host);
}

// パケットロスに関する統計データを表示
static void	print_stats_packet_loss(const t_stat_data* stat_data) {
	printf("%zu packets transmitted, ", stat_data->sent_icmps);
	printf("%zu packets received, ", stat_data->received_echo_replies);
	if (stat_data->sent_icmps > 0) {
		double gain_rate = (double)stat_data->received_echo_replies / stat_data->sent_icmps;
		printf("%d%% packet loss", (int)((1 - gain_rate) * 100));
	}
	printf("\n");
}

// レイテンシー(ラウンドトリップ)に関する統計データを表示
static void	print_stats_latency(const t_stat_data* stat_data) {
	// 受信パケットがない場合は統計データを出さない
	if (stat_data->received_echo_replies_with_ts == 0 || stat_data->roundtrip_times == NULL) { return; }
	double	min = get_rtt_min(stat_data);
	double	max = get_rtt_max(stat_data);
	double	avg = get_rtt_average(stat_data);
	double	stddev = get_rtt_stddev(stat_data);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", min, avg, max, stddev);
}

void	print_stats(const t_ping* ping) {
	print_stats_ribbon(ping);
	print_stats_packet_loss(&ping->target.stat_data);
	if (ping->prefs.sending_timestamp) {
		print_stats_latency(&ping->target.stat_data);
	}
}

// 受信データグラムの統計情報を記録する
double	record_received(t_ping* ping, const t_acceptance* acceptance) {
	t_stat_data*	stat_data = &ping->target.stat_data;

	// 受信したICMPデータグラムのタイムスタンプが利用可能なら, ラウンドトリップタイムを取得する
	const bool	is_echo_reply = acceptance->icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY;
	if (!is_echo_reply) { return -1; }
	if (!ping->prefs.sending_timestamp) { return -1; }
	double	rtt = record_roundtrip_time(stat_data, acceptance);
	if (rtt < 0) { return -1; }
	stat_data->received_echo_replies_with_ts += 1;
	return rtt;
}

static bool	is_sequence_duplicated(const t_ping* ping, uint16_t sequence) {
	size_t		byte_index = sequence / 8;
	uint32_t	bit_index = sequence % 8;
	return (ping->target.sequence_field[byte_index] & (1u << bit_index)) != 0;
}

static void	set_sequence_field(t_ping* ping, uint16_t sequence) {
	size_t		byte_index = sequence / 8;
	uint32_t	bit_index = sequence % 8;
	ping->target.sequence_field[byte_index] |= (1u << bit_index);
}

void	record_echo_reply(t_ping* ping, t_acceptance* acceptance) {
	t_stat_data*	stat_data = &ping->target.stat_data;
	stat_data->received_icmps += 1;

	const bool	is_echo_reply = acceptance->icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY;
	if (!is_echo_reply) { return ; }
	uint16_t	sequence = acceptance->icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ;
	if (is_sequence_duplicated(ping, sequence)) {
		DEBUGWARN("received duplicate echo reply: seq=%u", sequence);
		acceptance->is_duplicate = true;
		return ;
	}
	set_sequence_field(ping, sequence);
	stat_data->received_echo_replies += 1;
}

