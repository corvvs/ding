#include "ping.h"

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	g_interrupted = 1;
}

// 1つの宛先に対して ping セッションを実行する
int	ping_pong(t_ping* ping, const socket_address_in_t* addr_to) {

	// [初期出力]
	const size_t datagram_payload_len = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);
	printf("PING %s (%s): %zu data bytes\n",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);

	// [シグナルハンドラ設定]
	g_interrupted = 0;
	signal(SIGINT, sig_int);


	timeval_t	interval_request = {
		.tv_sec = 1,
		.tv_usec = 0,
	};
	timeval_t	now = get_current_time();
	timeval_t	last_request_sent = now;

	// 受信タイムアウトしたかどうか
	bool	receiving_timed_out = false;

	// [送受信ループ]
	uint16_t	sequence = 0;
	for (; g_interrupted == 0 ;) {
		if (ping->stat_data.packets_sent == 0 || receiving_timed_out) {
			receiving_timed_out = false;
			// [送信: Echo Request]
			// - まだ何も送信していない時
			// - 受信タイムアウトした時
			if (send_request(ping, addr_to, sequence) < 0) {
				break;
			}
			last_request_sent = get_current_time();
		} else {
			receiving_timed_out = false;
			// [受信: Echo Reply]

			// タイムアウト時刻を設定する
			now = get_current_time();
			timeval_t	recv_timeout = sub_times(&now, &last_request_sent);
			recv_timeout = sub_times(&interval_request, &recv_timeout);
			if (recv_timeout.tv_sec < 0 || recv_timeout.tv_usec < 0) {
				recv_timeout = (timeval_t){
					.tv_sec = 0,
					.tv_usec = 0,
				};
			}
			DEBUGINFO("recv_timeout: %.3fms", recv_timeout.tv_sec * 1000.0 + recv_timeout.tv_usec / 1000.0);
			if (setsockopt(ping->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) < 0) {
				perror("setsockopt failed");
				exit(EXIT_FAILURE);
			}

			t_acceptance	acceptance = {
				.recv_buffer_len = RECV_BUFFER_LEN,
			};
			{
				t_receipt_result rr = receive_reply(ping, &acceptance);
				if (rr == RR_TIMEOUT) {
					receiving_timed_out = true;
					continue;
				}
				if (rr == RR_ERROR) {
					break;
				}
			}
			if (check_acceptance(ping, &acceptance, addr_to)) {
				break;
			}
			const double triptime = mark_receipt(ping, &acceptance);
			// [受信時出力]
			printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
				acceptance.icmp_whole_len,
				ping->target.resolved_host,
				acceptance.icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
				acceptance.ip_header->ttl,
				triptime
			);
		}
	}

	// [最終出力]
	print_stats(ping);

	return 0;
}
