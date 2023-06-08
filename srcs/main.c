#include "ping.h"

int	g_is_little_endian;

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	g_interrupted = 1;
}

// 1つの宛先に対して ping セッションを実行する
int	run_ping_session(t_ping* ping, const socket_address_in_t* addr_to) {

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

int main(int argc, char **argv) {
	if (argc < 1) {
		return 1;
	}

	// NOTE: プログラム名は使用しない
	argc -= 1;
	argv += 1;

	// TODO: オプションの解析

	if (argc < 1) {
		printf("ping: missing host operand\n");
		return 64; // なぜ 64 なのか...
	}

	t_ping ping = {
		.target = {
			.given_host = *argv,
		},
		.icmp_header_id = getpid(),
	};

	g_is_little_endian = is_little_endian();
	DEBUGWARN("g_is_little_endian: %d", g_is_little_endian);

	// [ソケット作成]
	// ソケットは全宛先で使い回すので最初に生成する
	ping.socket_fd = create_icmp_socket();

	do {
		ping.stat_data = (t_stat_data){};
		socket_address_in_t	addr = {0};
		// [アドレス変換]
		if (retrieve_address_to(&ping, &addr)) {
			return -1;
		}

		// [エコー送信]
		run_ping_session(&ping, &addr);

		// [宛先単位の後処理]
		free(ping.stat_data.rtts);
	} while (0);

	// [全体の後処理]
	close(ping.socket_fd);
}
