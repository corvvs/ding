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

	// [受信時出力]
	const double triptime = record_received(ping, &acceptance);
	print_received(ping, &acceptance, triptime);
}
