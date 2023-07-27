#include "ping.h"

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	DEBUGOUT("received signal: %d", signal);
	g_interrupted = 1;
}

static bool	reached_ping_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.packets_sent >= ping->prefs.count;
}

static bool	reached_pong_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.packets_received >= ping->prefs.count;
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
	DEBUGOUT("count: %zu, sent: %zu, recv: %zu, timed_out: %d",
		ping->prefs.count,
		ping->target.stat_data.packets_sent,
		ping->target.stat_data.packets_received,
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
		t = sub_times(&t, &ping->start_time);
		if (ping->prefs.session_timeout_s * 1000.0 < get_ms(&t)) {
			DEBUGWARN("session timeout reached: " U64T, ping->prefs.session_timeout_s);
			return false;
		}
	}
	// Yes!!
	return true;
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

static void	print_sent(const t_ping* ping) {
	if (ping->prefs.flood) {
		ft_putchar_fd('.', STDOUT_FILENO);
	}
}

static void	print_received(
	const t_ping* ping,
	const t_acceptance* acceptance,
	bool sending_timestamp,
	double triptime
) {
	if (ping->prefs.flood) {
		ft_putchar_fd('\b', STDOUT_FILENO);
		return;
	}

	// 本体
	printf("%zu bytes from %s: icmp_seq=%u ttl=%u",
		acceptance->icmp_whole_len,
		stringify_address(&acceptance->ip_header->IP_HEADER_SRC),
		acceptance->icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
		acceptance->ip_header->IP_HEADER_TTL
	);
	if (sending_timestamp) {
		printf(" time=%.3f ms", triptime);
	}
	printf("\n");

	// もしあるならIPタイムスタンプを表示する
	print_ip_timestamp(ping, acceptance);
}

static void	print_epilogue(const t_ping* ping, bool sending_timestamp) {
	print_stats(ping, sending_timestamp);
}

// 1つの宛先に対して ping セッションを実行する
int	ping_pong(t_ping* ping) {
	const bool	sending_timestamp = ping->prefs.data_size >= sizeof(timeval_t);

	// [初期出力]
	print_prologue(ping);

	// [送受信ループ]
	uint16_t	sequence = 0;
	ping->start_time = get_current_time();

	// preload
	for (size_t n = 0; n < ping->prefs.preload; n += 1, sequence += 1) {
		if (send_request(ping, sequence) < 0) {
			break;
		}
	}

	// タイムアウトの計算
	const timeval_t	ping_interval = ping->prefs.flood
		? TV_PING_FLOOD_INTERVAL
		: TV_PING_DEFAULT_INTERVAL;
	timeval_t		now = get_current_time();
	timeval_t		last_request_sent = get_current_time();
	const timeval_t	receiving_timeout = {
		.tv_sec = ping->prefs.wait_after_final_request_s,
		.tv_usec = 0,
	};

	// 受信タイムアウトしたかどうか
	bool	receiving_timed_out = false;

	// [シグナルハンドラ設定]
	signal(SIGINT, sig_int);

	while (should_continue_session(ping, receiving_timed_out)) {
		if (can_send_ping(ping, receiving_timed_out)) {
			// [送信: Echo Request]
			receiving_timed_out = false;
			if (send_request(ping, sequence) < 0) {
				break;
			}
			print_sent(ping);
			last_request_sent = get_current_time();
			sequence += 1;
		} else {
			receiving_timed_out = false;
			// [受信: Echo Reply]

			// [タイムアウト時刻を設定する]
			now = get_current_time();
			const timeval_t	elapsed_from_last_ping = sub_times(&now, &last_request_sent);
			timeval_t	recv_timeout = sub_times(
				reached_ping_limit(ping) ? &receiving_timeout : &ping_interval,
				&elapsed_from_last_ping);
			if (recv_timeout.tv_sec < 0 || recv_timeout.tv_usec < 0) {
				recv_timeout = TV_NEARLY_ZERO; // ← 厳密にはゼロではない; ゼロを指定するとタイムアウトしなくなるため
			}
			DEBUGOUT("recv_timeout: %.3fms", get_ms(&recv_timeout));
			if (setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(timeval_t)) < 0) {
				exit_with_error(EXIT_FAILURE, errno, "setsockopt(RECV Timeout)");
			}

			// [受信とチェック]
			t_acceptance	acceptance = {
				.recv_buffer_len = RECV_BUFFER_LEN,
			};
			{
				const t_received_result rr = receive_reply(ping, &acceptance);
				if (rr == RR_TIMEOUT) {
					receiving_timed_out = true;
					continue;
				}
				if (rr == RR_INTERRUPTED) {
					continue;
				}
				if (rr == RR_ERROR) {
					break;
				}
			}
			if (check_acceptance(ping, &acceptance)) {
				continue;
			}

			// [受信時出力]
			const double triptime = sending_timestamp ? mark_received(ping, &acceptance) : 0;
			print_received(ping, &acceptance, sending_timestamp, triptime);
		}
	}

	// [シグナルハンドラ解除]
	signal(SIGINT, SIG_DFL);

	// [最終出力]
	print_epilogue(ping, sending_timestamp);

	return 0;
}
