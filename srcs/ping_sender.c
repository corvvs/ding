#include "ping.h"

extern int	g_is_little_endian;

// ICMP Echo Request を送信
int	send_request(t_ping* ping, uint16_t sequence) {
	const socket_address_in_t* addr = &ping->target.addr_to;
	uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
	construct_icmp_datagram(ping, datagram_buffer, sizeof(datagram_buffer), sequence);

	int rv = sendto(
		ping->socket_fd,
		datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE,
		0,
		(struct sockaddr *)addr, sizeof(socket_address_in_t)
	);
	DEBUGOUT("sendto rv: %d", rv);
	if (rv < 0) {
		DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
		return rv;
	}
	ping->target.stat_data.packets_sent += 1;
	return rv;
}
