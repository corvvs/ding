#include "ping.h"

int	g_is_little_endian;

// 与えられた宛先から sockaddr_in を生成する
socket_address_in_t	retrieve_addr(const t_ping* ping) {
	socket_address_in_t addr = {};
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ping->target.resolved_host, &addr.sin_addr);
	return addr;
}

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	g_interrupted = 1;
}

// 1つの宛先に対して ping セッションを実行する
int	run_ping_session(t_ping* ping, const socket_address_in_t* addr_to) {
	const size_t datagram_payload_len = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);
	// 送信前出力
	printf("PING %s (%s): %zu data bytes\n",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);

	g_interrupted = 0;
	signal(SIGINT, sig_int);

	// [ICMPヘッダを準備する]
	for (uint16_t	sequence = 0; g_interrupted == 0; sequence += 1) {

		uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
		deploy_datagram(ping, datagram_buffer, sizeof(datagram_buffer), sequence);

		// 送信!!
		if (send_ping(ping, datagram_buffer, sizeof(datagram_buffer), addr_to) < 0) {
			break;
		}
		// ECHO応答の受信を待機する
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
		// 受信時出力
		printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
			acceptance.icmp_whole_len,
			ping->target.resolved_host,
			acceptance.icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
			acceptance.ip_header->ttl,
			triptime
		);

		sleep(1);
	}
	// 統計情報を表示する
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

	{
		if (resolve_host(&ping.target)) {
			return -1;
		}
		DEBUGWARN("given:    %s", ping.target.given_host);
		DEBUGWARN("resolved: %s", ping.target.resolved_host);

		// [アドレス変換]
		socket_address_in_t	addr = retrieve_addr(&ping);

		// [エコー送信]
		run_ping_session(&ping, &addr);

		// [宛先単位の後処理]
		free(ping.stat_data.rtts);
		ping.stat_data = (t_stat_data){};
	}

	// [全体の後処理]
	close(ping.socket_fd);
}
