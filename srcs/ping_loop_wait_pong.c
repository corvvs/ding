#include "ping.h"

// [受信処理のタイムアウトを設定する]
static void	set_waiting_timeout(const t_ping* ping, const timeval_t* timeout) {
	const timeval_t	now = get_current_time();
	const timeval_t	elapsed_from_last_ping = sub_times(&now, &ping->target.last_request_sent);
	timeval_t		recv_timeout = sub_times(timeout, &elapsed_from_last_ping);
	if (recv_timeout.tv_sec < 0 || recv_timeout.tv_usec < 0) {
		recv_timeout = TV_NEARLY_ZERO; // ← 厳密にはゼロではない; ゼロを指定すると無限に待ってしまうため
	}
	DEBUGOUT("recv_timeout: %.3fms", get_ms(&recv_timeout));
	if (setsockopt(ping->socket, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(timeval_t)) < 0) {
		exit_with_error(EXIT_FAILURE, errno, "setsockopt(RECV Timeout)");
	}
}

static void	print_echo_reply(
	const t_ping* ping,
	const t_acceptance* acceptance,
	double triptime
) {
	const ip_header_t*	ip_header = acceptance->ip_header;
	icmp_header_t*		icmp_header = acceptance->icmp_header;
	const size_t		icmp_whole_len = acceptance->icmp_whole_len;
	printf("%zu bytes from %s: icmp_seq=%u ttl=%u",
		icmp_whole_len,
		stringify_address(&ip_header->IP_HEADER_SRC),
		icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
		ip_header->IP_HEADER_TTL
	);
	if (ping->sending_timestamp) {
		printf(" time=%.3f ms", triptime);
	}
	printf("\n");
}

static void	print_received(
	const t_ping* ping,
	const t_acceptance* acceptance,
	double triptime
) {
	if (ping->prefs.flood) {
		ft_putchar_fd('\b', STDOUT_FILENO);
		return;
	}

	switch (acceptance->icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY: {
			print_echo_reply(ping, acceptance, triptime);
			break;
		}
		case ICMP_TYPE_TIME_EXCEEDED: {
			print_time_exceeded(ping, acceptance);
			break;
		}
	}

	// もしあるならIPタイムスタンプを表示する
	print_ip_timestamp(ping, acceptance);
}

// [Echo Reply 待機処理]
void	ping_loop_wait_pong(t_ping* ping, const timeval_t* timeout) {
	t_session* target = &ping->target;

	target->receiving_timed_out = false;
	set_waiting_timeout(ping, timeout);

	// [受信とチェック]
	t_acceptance	acceptance = {
		.recv_buffer_len = RECV_BUFFER_LEN,
	};
	switch (receive_reply(ping, &acceptance)) {
		case RR_TIMEOUT:
			target->receiving_timed_out = true;
			return;
		case RR_ERROR:
			target->error_occurred = true;
			return;
		case RR_INTERRUPTED:
		case RR_UNACCEPTABLE:
			return;
		case RR_SUCCESS:
			break;
	}

	// [受信できた場合は出力]
	const double triptime = record_received(ping, &acceptance);
	print_received(ping, &acceptance, triptime);
}
