#include "ping.h"

extern int	g_is_little_endian;

// ICMP Echo Request を送信
int	send_request(t_ping* ping, uint16_t sequence) {
	const socket_address_in_t*	addr = &ping->target.address_to;
	uint8_t						datagram_buffer[MAX_IPV4_DATAGRAM_SIZE];
	const size_t				actual_datagram_size = sizeof(icmp_header_t) + ping->prefs.data_size;
	construct_icmp_datagram(ping, datagram_buffer, actual_datagram_size, sequence);

	int rv = sendto(
		ping->socket,
		datagram_buffer, actual_datagram_size,
		0,
		(socket_address_t*)addr, sizeof(socket_address_in_t)
	);
	DEBUGOUT("sendto rv: %d", rv);
	if (rv < 0) {
		dprintf(STDERR_FILENO, "%s: sending packet: %s\n", PROGRAM_NAME, strerror(errno));
		return rv;
	}

	ping->target.stat_data.sent_icmps += 1;
	if (ping->prefs.hexdump_sent) {
		debug_hexdump("sent message", datagram_buffer, actual_datagram_size);
	}
	return rv;
}
