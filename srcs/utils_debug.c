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

void	debug_msg_flags(const struct msghdr* msg) {
	// msg_flags の表示
	DEBUGINFO("msg_flags: %x",    msg->msg_flags);
	DEBUGINFO("MSG_EOR: %d",      !!(msg->msg_flags & MSG_EOR)); // レコードの終り
	DEBUGINFO("MSG_TRUNC: %d",    !!(msg->msg_flags & MSG_TRUNC)); // データグラムを受信しきれなかった
	DEBUGINFO("MSG_CTRUNC: %d",   !!(msg->msg_flags & MSG_CTRUNC)); // 補助データが受信しきれなかった
	DEBUGINFO("MSG_OOB: %d",      !!(msg->msg_flags & MSG_OOB)); // 対域外データを受信した
	// DEBUGINFO("MSG_ERRQUEUE: %d", !!(msg->msg_flags & MSG_ERRQUEUE)); // ソケットのエラーキューからエラーを受信した
}

void	debug_ip_header(const void* mem) {
	const ip_header_t*	ip_hd = (const ip_header_t*) mem;
	dprintf(STDERR_FILENO, "ip_header_t:\n");
	dprintf(STDERR_FILENO, "  ip_v: %u\n", ip_hd->IP_HEADER_VER);
	dprintf(STDERR_FILENO, "  ip_hl: %u(-> %u)\n", ip_hd->IP_HEADER_HL, ip_hd->IP_HEADER_HL * 4);
	dprintf(STDERR_FILENO, "  ip_tos: %u(%X)\n", ip_hd->IP_HEADER_TOS, ip_hd->IP_HEADER_TOS);
	dprintf(STDERR_FILENO, "  ip_len: %u(%X)\n", ip_hd->IP_HEADER_LEN, ip_hd->IP_HEADER_LEN);
	dprintf(STDERR_FILENO, "  ip_id: %u\n", ip_hd->IP_HEADER_ID);
	dprintf(STDERR_FILENO, "  ip_off: %u\n", ip_hd->IP_HEADER_OFF);
	dprintf(STDERR_FILENO, "  ip_ttl: %u\n", ip_hd->IP_HEADER_TTL);
	dprintf(STDERR_FILENO, "  ip_p: %u\n", ip_hd->IP_HEADER_PROT);
	dprintf(STDERR_FILENO, "  ip_sum: %u(%X)\n", ip_hd->IP_HEADER_SUM, ip_hd->IP_HEADER_SUM);
	dprintf(STDERR_FILENO, "  ip_src: %s\n", stringify_address(&ip_hd->IP_HEADER_SRC));
	dprintf(STDERR_FILENO, "  ip_dst: %s\n", stringify_address(&ip_hd->IP_HEADER_DST));
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
