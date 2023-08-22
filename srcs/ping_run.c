#include "ping.h"

static int	create_socket(bool* inaccessible_ipheader, const t_preferences* pref) {
	int sock = create_icmp_socket(inaccessible_ipheader, pref);
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

static void	run_sessions(t_ping* ping, char** hosts) {
	for (; *hosts != NULL; hosts += 1) {
		const char*	given_host = *hosts;
		DEBUGOUT("<start session for \"%s\">", given_host);

		ping->target.stat_data = (t_stat_data){ .roundtrip_times = NULL };

		if (!setup_target_from_host(given_host, &ping->target)) {
			break;
		}

		ping_session(ping);

		free(ping->target.stat_data.roundtrip_times);
	};
}

static bool	received_ipheader_modified(void) {
#ifdef __APPLE__
	return true;
#else
	return false;
#endif
}

int		ping_run(const t_preferences* preference, char** hosts) {
	if (*hosts == NULL) {
		print_error_by_message("missing host operand");
		return STATUS_OPERAND_FAILED; // なぜ 64 なのか
	}

	// ソケットは全宛先で使い回すので最初に生成する
	bool	inaccessible_ipheader = false;
	int sock = create_socket(&inaccessible_ipheader, preference);
	if (sock < 0) {
		return STATUS_GENERIC_FAILED;
	}

	t_ping ping = {
		.socket						= sock,
		.icmp_header_id				= getpid(),
		.prefs						= *preference,
		.inaccessible_ipheader		= inaccessible_ipheader,
		.received_ipheader_modified	= received_ipheader_modified(),
	};

	run_sessions(&ping, hosts);

	close(ping.socket); // 社会人としてのマナー
	return STATUS_SUCCEEDED;
}
