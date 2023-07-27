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
	int parsed_options = parse_option(argc, argv, by_root, &pref);
	if (parsed_options < 0) {
		return 64;
	}

	// もし show_usage が有効になっている場合 usage を表示して終了する
	if (pref.show_usage) {
		print_usage();
		return 0;
	}

	argc -= parsed_options;
	argv += parsed_options;

	if (argc < 1) {
		print_error_by_message("missing host operand");
		return 64; // なぜ 64 なのか...
	}

	t_ping ping = {
		.icmp_header_id = getpid(),
		.prefs = pref,
		// ICMP データサイズが timeval_t のサイズ以上なら, ICMP Echo にタイムスタンプを載せる
		.sending_timestamp = pref.data_size >= sizeof(timeval_t),
	};

	// [ソケット作成]
	// ソケットは全宛先で使い回すので最初に生成する
	int sock = create_icmp_socket(&ping.prefs);
	if (sock < 0) {
		return 1;
	}
	ping.socket_fd = sock;
	// ソケット作成後, もしルート権限で実行されていたら, その権限を放棄する
	if (setuid(getuid())) { // ← 実ユーザIDがルートでないなら, これでルート権限が外れる
		print_special_error_by_errno("setuid");
		return 1;
	}

	for (; argc > 0; argc--, argv++) {
		const char*	given_host = *argv;
		DEBUGOUT("<start session for \"%s\">", given_host);

		ping.target.stat_data = (t_stat_data){};

		if (setup_target_from_host(given_host, &ping.target)) {
			break;
		}

		ping_pong(&ping);

		free(ping.target.stat_data.rtts);
	};
	DEBUGWARN("%s", "finished");
}
