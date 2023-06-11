#include "ping.h"

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	DEBUGOUT("received signal: %d", signal);
	g_interrupted = 1;
}

static bool	reached_ping_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->stat_data.packets_sent >= ping->prefs.count;
}

static bool	reached_pong_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->stat_data.packets_received >= ping->prefs.count;
}

static bool	should_continue_pinging(const t_ping* ping, bool receiving_timed_out) {
	// 割り込みがあった -> No
	if (g_interrupted) {
		DEBUGWARN("%s", "interrupted");
		return false;
	}
	DEBUGOUT("count: %zu, sent: %zu, recv: %zu, timed_out: %d",
		ping->prefs.count, ping->stat_data.packets_sent, ping->stat_data.packets_received, receiving_timed_out);
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
			DEBUGWARN("session timeout reached: %lu", ping->prefs.session_timeout_s);
			return false;
		}
	}
	// Yes!!
	return true;
}

// 1つの宛先に対して ping セッションを実行する
int	ping_pong(t_ping* ping) {
	const socket_address_in_t* addr_to = &ping->target.addr_to;
	// [初期出力]
	const size_t datagram_payload_len = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);
	printf("PING %s (%s): %zu data bytes",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);
	if (ping->prefs.verbose) {
		printf(", id = 0x%04x", ping->icmp_header_id);
	}
	printf("\n");
	// [シグナルハンドラ設定]
	g_interrupted = 0; // TODO: セッションごとにリセット**しない**
	signal(SIGINT, sig_int);

	// タイムアウトの計算
	timeval_t	interval_request = {
		.tv_sec = 1,
		.tv_usec = 0,
	};

	// 受信タイムアウトしたかどうか
	bool	receiving_timed_out = false;

	// [送受信ループ]
	uint16_t	sequence = 0;
	ping->start_time = get_current_time();

	// preload
	for (size_t n = 0; n < ping->prefs.preload; n += 1, sequence += 1) {
		if (send_request(ping, addr_to, sequence) < 0) {
			break;
		}
	}
	timeval_t		now = get_current_time();
	timeval_t		last_request_sent = get_current_time();
	const timeval_t	receiving_timeout = {
		.tv_sec = ping->prefs.wait_after_final_request_s,
		.tv_usec = 0,
	};

	while (should_continue_pinging(ping, receiving_timed_out)) {
		if (ping->stat_data.packets_sent == 0 || (!reached_ping_limit(ping) && receiving_timed_out)) {
			// [送信: Echo Request]
			receiving_timed_out = false;
			if (send_request(ping, addr_to, sequence) < 0) {
				break;
			}
			last_request_sent = get_current_time();
			sequence += 1;
		} else {
			receiving_timed_out = false;
			// [受信: Echo Reply]

			// [タイムアウト時刻を設定する]
			now = get_current_time();
			const timeval_t	elapsed_from_last_ping = sub_times(&now, &last_request_sent);
			timeval_t	recv_timeout = sub_times(
				reached_ping_limit(ping) ? &receiving_timeout : &interval_request,
				&elapsed_from_last_ping);
			if (recv_timeout.tv_sec < 0 || recv_timeout.tv_usec < 0) {
				DEBUGOUT("recv_timeout: %.3fms -> zeroize", get_ms(&recv_timeout));
				recv_timeout = (timeval_t){
					.tv_sec = 0, .tv_usec = 1000,
				};
			}
			DEBUGOUT("recv_timeout: %.3fms", get_ms(&recv_timeout));
			if (setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) < 0) {
				perror("setsockopt failed");
				exit(EXIT_FAILURE);
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
			if (check_acceptance(ping, &acceptance, addr_to)) {
				continue;
			}

			// [受信時出力]
			const double triptime = mark_received(ping, &acceptance);
			printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
				acceptance.icmp_whole_len,
				stringify_address(&acceptance.ip_header->IP_HEADER_SRC),
				acceptance.icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
				acceptance.ip_header->IP_HEADER_TTL,
				triptime
			);
		}
	}

	// [最終出力]
	print_stats(ping);

	return 0;
}
