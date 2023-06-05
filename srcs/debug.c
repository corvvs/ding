#include "ping.h"
extern int	g_is_little_endian;

void	debug_hexdump(const char* label, const void* mem, size_t len) {
	uint8_t*	vmem = (uint8_t*) mem;
	dprintf(STDERR_FILENO, "%s:\n", label);
	for (size_t i = 0; i < len; ++i) {
		if (i != 0 && i % 16 == 0) {
			dprintf(STDERR_FILENO, "\n");
		}
		dprintf(STDERR_FILENO, "%02x ", vmem[i]);
	}
	dprintf(STDERR_FILENO, "\n");
}

void	debug_ip_header(const void* mem) {
	const ip_header_t*	ip_hd = (const ip_header_t*) mem;
	dprintf(STDERR_FILENO, "ip_header_t:\n");
	dprintf(STDERR_FILENO, "  ip_v: %u\n", ip_hd->version);
	dprintf(STDERR_FILENO, "  ip_hl: %u\n", ip_hd->ihl);
	dprintf(STDERR_FILENO, "  ip_tos: %u\n", ip_hd->tos);
	dprintf(STDERR_FILENO, "  ip_len: %u(%X)\n", ip_hd->tot_len, ip_hd->tot_len);
	dprintf(STDERR_FILENO, "  ip_id: %u\n", ip_hd->id);
	dprintf(STDERR_FILENO, "  ip_off: %u\n", ip_hd->frag_off);
	dprintf(STDERR_FILENO, "  ip_ttl: %u\n", ip_hd->ttl);
	dprintf(STDERR_FILENO, "  ip_p: %u\n", ip_hd->protocol);
	dprintf(STDERR_FILENO, "  ip_sum: %u(%X)\n", ip_hd->check, ip_hd->check);
	dprintf(STDERR_FILENO, "  ip_src: %u.%u.%u.%u\n",
		(ip_hd->saddr >> 24) & 0xff,
		(ip_hd->saddr >> 16) & 0xff,
		(ip_hd->saddr >> 8) & 0xff,
		(ip_hd->saddr >> 0) & 0xff
	);
	dprintf(STDERR_FILENO, "  ip_dst: %u.%u.%u.%u\n",
		(ip_hd->daddr >> 24) & 0xff,
		(ip_hd->daddr >> 16) & 0xff,
		(ip_hd->daddr >> 8) & 0xff,
		(ip_hd->daddr >> 0) & 0xff
	);
}

void	debug_icmp_header(const void* mem) {
	const icmp_header_t*	hd = (const icmp_header_t*) mem;
	dprintf(STDERR_FILENO, "icmp_header_t:\n");
	dprintf(STDERR_FILENO, "  Type: %u\n", hd->ICMP_HEADER_TYPE);
	dprintf(STDERR_FILENO, "  Code: %u\n", hd->ICMP_HEADER_CODE);
	dprintf(STDERR_FILENO, "  CheckSum: %u\n", hd->ICMP_HEADER_CHECKSUM);
	dprintf(STDERR_FILENO, "  Echo.Id: %u(%X)\n", hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID, hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID);
	dprintf(STDERR_FILENO, "  Echo.Sequence: %u(%X)\n", hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ, hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ);
}
