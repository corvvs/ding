#include "ping.h"

int	g_is_little_endian;

int main(int argc, char **argv) {
	g_is_little_endian = is_little_endian();
	if (argc < 1) {
		return 1;
	}
	// NOTE: プログラム名は使用しない
	argc -= 1;
	argv += 1;

	// オプションの解析
	t_preferences	pref = default_preferences();
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

	t_ping ping = {
		.icmp_header_id = getpid(),
		.prefs = pref,
	};

	// [ソケット作成]
	// ソケットは全宛先で使い回すので最初に生成する
	ping.socket_fd = create_icmp_socket(&ping.prefs);

	for (; argc > 0; argc--, argv++) {
		const char*	given_host = *argv;
		DEBUGOUT("<start session for \"%s\">", given_host);
		// [アドレス変換]
		if (retrieve_target(given_host, &ping.target)) {
			break;
		}

		// [エコー送信]
		ping.stat_data = (t_stat_data){};
		ping_pong(&ping);

		// [宛先単位の後処理]
		free(ping.stat_data.rtts);
	} while (0);

	// [全体の後処理]
	close(ping.socket_fd);
}
