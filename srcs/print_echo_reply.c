#include "ping.h"

void	print_echo_reply(
	const t_ping* ping,
	const t_acceptance* acceptance,
	double triptime
) {
	const ip_header_t*	ip_header = acceptance->ip_header;
	icmp_header_t*		icmp_header = acceptance->icmp_header;
	const size_t		icmp_datagram_size = acceptance->icmp_datagram_size;
	printf("%zu bytes from %s: icmp_seq=%u ttl=%u",
		icmp_datagram_size,
		stringify_address(&ip_header->IP_HEADER_SRC),
		icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
		ip_header->IP_HEADER_TTL
	);
	if (ping->prefs.sending_timestamp && triptime >= 0) {
		printf(" time=%.3f ms", triptime);
	}
	if (acceptance->is_duplicate) {
		printf(" (DUP!)");
	}
	printf("\n");
}
