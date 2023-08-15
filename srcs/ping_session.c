#include "ping.h"

static void	print_prologue(const t_ping* ping) {
	const size_t datagram_payload_len = ping->prefs.data_size;
	printf("PING %s (%s): %zu data bytes",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);
	if (ping->prefs.verbose) {
		printf(", id = 0x%04x", ping->icmp_header_id);
	}
	printf("\n");
}

static void	print_epilogue(const t_ping* ping) {
	print_stats(ping);
}

// 1つの宛先に対して ping セッションを実行する
int	ping_session(t_ping* ping) {

	// [初期出力]
	print_prologue(ping);

	t_session* target = &ping->target;
	target->start_time = get_current_time();

	// preload
	for (size_t n = 0; n < ping->prefs.preload; n += 1, target->icmp_sequence += 1) {
		if (send_request(ping, target->icmp_sequence) < 0) {
			break;
		}
	}

	// [主送受信ループ]
	ping_loop(ping);

	// [最終出力]
	print_epilogue(ping);

	// TODO: 常に0を返すくらいなら void にするか?
	return 0;
}
