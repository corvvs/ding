#include "ping.h"

int	g_is_little_endian;

static bool	exec_by_root(void) {
	return getuid() == 0;
}

int main(int argc, char **argv) {
	g_is_little_endian = is_little_endian();
	if (argc < 1) {
		return 1;
	}
	// NOTE: プログラム名は使用しない
	argc -= 1;
	argv += 1;

	// オプションの解析
	const bool		by_root = exec_by_root();
	t_preferences	pref = default_preferences();
	int parsed = parse_option(argc, argv, by_root, &pref);
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
	// ソケット作成後, もしルート権限で実行されていたら, その権限を放棄する
	if (setuid(getuid())) { // ← 実ユーザIDがルートでないなら, これでルート権限が外れる
		perror("setuid");
		return 1;
	}

	for (; argc > 0; argc--, argv++) {
		const char*	given_host = *argv;
		ping.target.stat_data = (t_stat_data){};
		DEBUGOUT("<start session for \"%s\">", given_host);
		// [アドレス変換]
		if (retrieve_target(given_host, &ping.target)) {
			break;
		}

		// [エコー送信]
		ping_pong(&ping);

		// [宛先単位の後処理]
		free(ping.target.stat_data.rtts);
	};
}
