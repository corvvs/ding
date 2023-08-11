#include "ping.h"

static void	print_sent(const t_ping* ping) {
	if (ping->prefs.flood) {
		ft_putchar_fd('.', STDOUT_FILENO);
	}
}

// [送信: Echo Request]
void	ping_loop_send_ping(t_ping* ping) {
	// TODO: 単一責務になってないかもしれない
	t_session* target = &ping->target;

	target->receiving_timed_out = false;
	if (send_request(ping, target->icmp_sequence) < 0) {
		target->error_occurred = true;
		return;
	}

	print_sent(ping);
	target->last_request_sent = get_current_time();
	target->icmp_sequence += 1;
}
