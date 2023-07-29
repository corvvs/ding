#include "ping.h"

int	g_is_little_endian;

static bool	exec_by_root(void) {
	return getuid() == 0;
}

static int	set_preference(t_arguments* args, t_preferences* pref) {
	const bool		by_root = exec_by_root();
	int parsed_options = parse_option(args, by_root, pref);
	if (parsed_options < 0) {
		return -1;
	}
	return 0;
}

static int	create_socket(bool* socket_is_dgram, const t_preferences* pref) {
	int sock = create_icmp_socket(socket_is_dgram, pref);
	if (sock < 0) {
		return sock;
	}
	// ソケット作成後, もしルート権限で実行されていたら, その権限を放棄する
	if (setuid(getuid())) { // ← 実ユーザIDがルートでないなら, これでルート権限が外れる
		print_special_error_by_errno("setuid");
		close(sock);
		return -1;
	}
	return sock;
}

static void	run_ping_sessions(t_arguments* args, t_ping* ping) {
	for (; args->argc > 0; proceed_arguments(args, 1)) {
		const char*	given_host = *args->argv;
		DEBUGOUT("<start session for \"%s\">", given_host);

		ping->target.stat_data = (t_stat_data){ .rtts = NULL };

		if (setup_target_from_host(given_host, &ping->target)) {
			break;
		}

		ping_pong(ping);

		free(ping->target.stat_data.rtts);
	};
}

int main(int argc, char **argv) {
	g_is_little_endian = is_little_endian();
	t_arguments args = { .argc = argc, .argv = argv };
	if (args.argc < 1) {
		return STATUS_GENERIC_FAILED;
	}
	// NOTE: プログラム名は使用しないので飛ばす
	proceed_arguments(&args, 1);

	t_preferences	pref = default_preferences();
	if (set_preference(&args, &pref)) {
		return STATUS_OPERAND_FAILED;
	}
	if (pref.show_usage) {
		print_usage();
		return STATUS_SUCCEEDED;
	}

	if (args.argc < 1) {
		print_error_by_message("missing host operand");
		return STATUS_OPERAND_FAILED; // なぜ 64 なのか...
	}

	// ソケットは全宛先で使い回すので最初に生成する
	bool	socket_is_dgram = false;
	int sock = create_socket(&socket_is_dgram, &pref);
	if (sock < 0) {
		return STATUS_GENERIC_FAILED;
	}

	t_ping ping = {
		.socket_fd = sock,
		.icmp_header_id = getpid(),
		.prefs = pref,
		// ICMP データサイズが timeval_t のサイズ以上なら, ICMP Echo にタイムスタンプを載せる
		.sending_timestamp = pref.data_size >= sizeof(timeval_t),
	};
	run_ping_sessions(&args, &ping);
	close(ping.socket_fd); // 社会人としてのマナー
	DEBUGWARN("%s", "finished");
}
