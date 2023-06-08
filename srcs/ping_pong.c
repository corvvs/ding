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

	// [送受信ループ]
	for (uint16_t	sequence = 0; g_interrupted == 0; sequence += 1) {

		// [送信: Echo Request]
		if (send_request(ping, addr_to, sequence) < 0) {
			break;
		}

		// [受信: Echo Reply]
		t_acceptance	acceptance = {
			.recv_buffer_len = RECV_BUFFER_LEN,
		};
		if (receive_reply(ping, &acceptance) <= 0) {
			break;
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

		sleep(1);
	}

	// [最終出力]
	print_stats(ping);

	return 0;
}
