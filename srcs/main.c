#include "ping.h"

int	g_is_little_endian;

int main(int argc, char **argv) {
	if (argc < 1) {
		return 1;
	}
	// NOTE: プログラム名は使用しない
	argc -= 1;
	argv += 1;

	// オプションの解析
	t_preferences	pref = {0};
	int parsed = parse_option(argc, argv, &pref);
	if (parsed < 0) {
		return 64;
	}
	argc -= parsed;
	argv += parsed;

	if (argc < 1) {
		printf("ping: missing host operand\n");
		return 64; // なぜ 64 なのか...
	}

	DEBUGOUT("argv: %s", *argv);
	t_ping ping = {
		.target = {
			.given_host = *argv,
		},
		.icmp_header_id = getpid(),
		.prefs = pref,
	};

	g_is_little_endian = is_little_endian();
	DEBUGWARN("g_is_little_endian: %d", g_is_little_endian);

	// [ソケット作成]
	// ソケットは全宛先で使い回すので最初に生成する
	ping.socket_fd = create_icmp_socket();

	do {
		DEBUGOUT("%s", "<start session>");
		ping.stat_data = (t_stat_data){};
		socket_address_in_t	addr = {0};
		// [アドレス変換]
		if (retrieve_address_to(&ping, &addr)) {
			return -1;
		}

		// [エコー送信]
		ping_pong(&ping, &addr);

		// [宛先単位の後処理]
		free(ping.stat_data.rtts);
	} while (0);

	// [全体の後処理]
	close(ping.socket_fd);
}
