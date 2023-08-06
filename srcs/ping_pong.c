#include "ping.h"

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	DEBUGOUT("received signal: %d", signal);
	g_interrupted = 1;
}

static void	print_prologue(const t_ping* ping) {
	const size_t datagram_payload_len = ping->prefs.data_size;
	printf("PING %s (%s): %zu data bytes",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);
	if (ping->prefs.verbose) {
		printf(", id = 0x%04x", ping->icmp_header_id);
	}
	printf("\n");
}

static bool	reached_ping_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.packets_sent >= ping->prefs.count;
}

static bool	reached_pong_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.packets_received_any >= ping->prefs.count;
}

static bool	can_send_ping(const t_ping* ping, bool receiving_timed_out) {
	return ping->target.stat_data.packets_sent == 0 || (!reached_ping_limit(ping) && receiving_timed_out);
}

static bool	should_continue_session(const t_ping* ping, bool receiving_timed_out) {
	// 割り込みがあった -> No
	if (g_interrupted) {
		DEBUGWARN("%s", "interrupted");
		return false;
	}
	if (ping->target.error_occurred) {
		return false;
	}
	DEBUGOUT("count: %zu, sent: %zu, recv: %zu, recv_any: %zu, timed_out: %d",
		ping->prefs.count,
		ping->target.stat_data.packets_sent,
		ping->target.stat_data.packets_received,
		ping->target.stat_data.packets_received_any,
		receiving_timed_out);
	// カウントが設定されている and 受信数がカウント以上に達した and タイムアウトした
	if (reached_ping_limit(ping) && receiving_timed_out) {
		DEBUGWARN("ping limit reached: %zu", ping->prefs.count);
		return false;
	}
	if (reached_pong_limit(ping)) {
		DEBUGWARN("pong limit reached: %zu", ping->prefs.count);
		return false;
	}
	
	if (ping->prefs.session_timeout_s > 0) {
		timeval_t	t = get_current_time();
		t = sub_times(&t, &ping->target.start_time);
		if (ping->prefs.session_timeout_s * 1000.0 < get_ms(&t)) {
			DEBUGWARN("session timeout reached: " U64T, ping->prefs.session_timeout_s);
			return false;
		}
	}
	// Yes!!
	return true;
}

static void	print_sent(const t_ping* ping) {
	if (ping->prefs.flood) {
		ft_putchar_fd('.', STDOUT_FILENO);
	}
}

// [送信: Echo Request]
static void	send_ping(t_ping* ping) {
	t_session* target = &ping->target;
	target->receiving_timed_out = false;
	if (send_request(ping, target->icmp_sequence) < 0) {
		target->error_occurred = true;
		return;
	}
	print_sent(ping);
	target->last_request_sent = get_current_time();
	target->icmp_sequence += 1;
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

	const ip_header_t*	ip_header = acceptance->ip_header;
	icmp_header_t*		icmp_header = acceptance->icmp_header;
	const size_t		icmp_whole_len = acceptance->icmp_whole_len;
	switch (acceptance->icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY: {
			// 本体
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
			break;
		}
		case ICMP_TYPE_TIME_EXCEEDED: {
			icmp_detailed_header_t*	dicmp = (icmp_detailed_header_t*)icmp_header;
			ip_header_t*			original_ip = (ip_header_t*)&(dicmp->ICMP_DHEADER_ORIGINAL_IP);
			const size_t			original_ip_whole_len = icmp_whole_len - ((void*)original_ip - (void*)dicmp);
			const size_t			original_ip_header_len = original_ip->IP_HEADER_HL * 4;
			const size_t			original_icmp_whole_len = original_ip_whole_len - original_ip_header_len;
			// NOTE: ペイロードのオリジナルICMPの中身はカットされる可能性がある
			// (RFCでは"64 bits of Data Datagram"としか書かれていない)
			// ので, チェックサムは検証しない
			if (!ping->received_ipheader_modified) {
				flip_endian_ip(original_ip);
			}

			print_time_exceeded_line(ping, ip_header, original_ip, icmp_whole_len, original_icmp_whole_len);
			break;
		}
	}

	// もしあるならIPタイムスタンプを表示する
	print_ip_timestamp(ping, acceptance);
}

// [受信処理のタイムアウトを設定する]
static void	set_waiting_timeout(const t_ping* ping, const timeval_t* timeout) {
	const timeval_t	now = get_current_time();
	const timeval_t	elapsed_from_last_ping = sub_times(&now, &ping->target.last_request_sent);
	timeval_t		recv_timeout = sub_times(timeout, &elapsed_from_last_ping);
	if (recv_timeout.tv_sec < 0 || recv_timeout.tv_usec < 0) {
		recv_timeout = TV_NEARLY_ZERO; // ← 厳密にはゼロではない; ゼロを指定すると無限に待ってしまうため
	}
	DEBUGOUT("recv_timeout: %.3fms", get_ms(&recv_timeout));
	if (setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(timeval_t)) < 0) {
		exit_with_error(EXIT_FAILURE, errno, "setsockopt(RECV Timeout)");
	}
}

// [Echo Reply 待機処理]
static void	wait_pong(t_ping* ping, const timeval_t* timeout) {
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
		default:
			break;
	}

	// [受信時出力]
	const double triptime = mark_received(ping, &acceptance);
	print_received(ping, &acceptance, triptime);
}

static void	print_epilogue(const t_ping* ping) {
	print_stats(ping);
}

// 1つの宛先に対して ping セッションを実行する
int	ping_pong(t_ping* ping) {

	// [初期出力]
	print_prologue(ping);

	// [送受信ループ]
	t_session* target = &ping->target;
	target->start_time = get_current_time();

	// preload
	for (size_t n = 0; n < ping->prefs.preload; n += 1, target->icmp_sequence += 1) {
		if (send_request(ping, target->icmp_sequence) < 0) {
			break;
		}
	}

	// タイムアウトの計算
	const timeval_t	ping_interval = ping->prefs.flood
		? TV_PING_FLOOD_INTERVAL
		: TV_PING_DEFAULT_INTERVAL;
	const timeval_t	receiving_timeout = {
		.tv_sec = ping->prefs.wait_after_final_request_s,
		.tv_usec = 0,
	};
	target->last_request_sent = get_current_time();

	// [シグナルハンドラ設定]
	signal(SIGINT, sig_int);

	while (should_continue_session(ping, target->receiving_timed_out)) {
		if (can_send_ping(ping, target->receiving_timed_out)) {
			send_ping(ping);
		} else {
			const timeval_t*	timeout = reached_ping_limit(ping)
				? &receiving_timeout
				: &ping_interval;
			wait_pong(ping, timeout);
		}
	}

	// [シグナルハンドラ解除]
	signal(SIGINT, SIG_DFL);

	// [最終出力]
	print_epilogue(ping);

	return 0;
}
