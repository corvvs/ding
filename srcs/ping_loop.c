#include "ping.h"

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	DEBUGOUT("received signal: %d", signal);
	g_interrupted = 1;
}

static bool	reached_ping_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.sent_icmps >= ping->prefs.count;
}

static bool	reached_pong_limit(const t_ping* ping) {
	return ping->prefs.count > 0 && ping->target.stat_data.received_icmps >= ping->prefs.count;
}

static bool	can_send_ping(const t_ping* ping, bool receiving_timed_out) {
	return ping->target.stat_data.sent_icmps == 0 || (!reached_ping_limit(ping) && receiving_timed_out);
}

static bool	should_continue_session(const t_ping* ping, bool receiving_timed_out) {
	// 割り込みがあった -> No
	if (g_interrupted) {
		DEBUGWARN("%s", "interrupted");
		return false;
	}
	if (ping->target.error_occurred) {
		return false;
	}
	DEBUGOUT("count: %zu, sent: %zu, recv: %zu, recv_any: %zu, timed_out: %d",
		ping->prefs.count,
		ping->target.stat_data.sent_icmps,
		ping->target.stat_data.received_echo_replies_with_ts,
		ping->target.stat_data.received_icmps,
		receiving_timed_out);
	// カウントが設定されている and 受信数がカウント以上に達した and タイムアウトした
	if (reached_ping_limit(ping) && receiving_timed_out) {
		DEBUGWARN("ping limit reached: %zu", ping->prefs.count);
		return false;
	}
	if (reached_pong_limit(ping)) {
		DEBUGWARN("pong limit reached: %zu", ping->prefs.count);
		return false;
	}
	
	if (ping->prefs.session_timeout_s > 0) {
		timeval_t	t = get_current_time();
		t = sub_times(&t, &ping->target.start_time);
		if (ping->prefs.session_timeout_s * 1000.0 < get_ms(&t)) {
			DEBUGWARN("session timeout reached: " U64T, ping->prefs.session_timeout_s);
			return false;
		}
	}
	// Yes!!
	return true;
}

// [送受信ループ]
void	ping_loop(t_ping* ping) {
	// [シグナルハンドラ設定]
	signal(SIGINT, sig_int);

	t_session*		target = &ping->target;
	// タイムアウトの計算
	const timeval_t	ping_interval = ping->prefs.flood
		? TV_PING_FLOOD_INTERVAL
		: TV_PING_DEFAULT_INTERVAL;
	const timeval_t	receiving_timeout = {
		.tv_sec = ping->prefs.wait_after_final_request_s,
		.tv_usec = 0,
	};
	target->last_request_sent = get_current_time();

	// ループ開始
	// TODO: 複雑?
	while (should_continue_session(ping, target->receiving_timed_out)) {
		if (can_send_ping(ping, target->receiving_timed_out)) {
			ping_loop_send_ping(ping);
		} else {
			const timeval_t*	timeout = reached_ping_limit(ping)
				? &receiving_timeout
				: &ping_interval;
			ping_loop_wait_pong(ping, timeout);
		}
	}

	// [シグナルハンドラ解除]
	signal(SIGINT, SIG_DFL);
}
