#include "ping.h"
extern int	g_is_little_endian;

void	print_unexpected_icmp(const t_ping* ping, t_acceptance* acceptance) {
	(void)ping;
	(void)acceptance;
	// const ip_header_t*	ip_header = acceptance->ip_header;
	// icmp_header_t*		icmp_header = acceptance->icmp_header;
	// const size_t		icmp_whole_len = acceptance->icmp_whole_len;
}

// IPヘッダをダンプする
static void	print_original_ip_dump(const ip_header_t* ip_header) {
	const size_t	ip_header_len = ip_header->IP_HEADER_HL * 4;
	printf("IP Hdr Dump:\n");
	for (size_t i = 0; i < ip_header_len; i += 1) {
		if (i % 2 == 0) {
			printf(" ");
		}
		printf("%02x", ((uint8_t*)ip_header)[i]);
	}
	printf("\n");
}

// IPヘッダをテーブル表示する
static void	print_original_ip_table(const ip_header_t* ip_header) {
	// どうやら列ごとに右詰めらしい

	// print schema
	uint16_t	flag_offset = SWAP_NEEDED(ip_header->IP_HEADER_OFF);
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	printf("%2x", ip_header->IP_HEADER_VER);
	printf(" %2x", ip_header->IP_HEADER_HL);
	printf("  %02x", ip_header->IP_HEADER_TOS);
	printf(" %04x", (uint16_t)SWAP_NEEDED(ip_header->IP_HEADER_LEN));
	printf(" %04x", (uint16_t)SWAP_NEEDED(ip_header->IP_HEADER_ID));
	printf("   %1x", (flag_offset & 0xe000) >> 13);
	printf(" %04x", flag_offset & 0x1fff);
	printf("  %02x", ip_header->IP_HEADER_TTL);
	printf("  %02x", ip_header->IP_HEADER_PROT);
	printf(" %04x", (uint16_t)SWAP_NEEDED(ip_header->IP_HEADER_SUM));
	printf(" %s ", stringify_address(&ip_header->IP_HEADER_SRC));
	printf(" %s ", stringify_address(&ip_header->IP_HEADER_DST));
	printf("\n");
}

// ICMPヘッダをライン表示する
static void	print_original_icmp_table(const icmp_header_t* icmp_header, size_t original_icmp_whole_len) {
	printf("ICMP: type %u, code %u, size %zu",
		icmp_header->ICMP_HEADER_TYPE,
		icmp_header->ICMP_HEADER_CODE,
		original_icmp_whole_len
	);
	if (icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REQUEST ||
		icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY) {
		printf(", id 0x%04x, seq 0x%04x",
			(uint16_t)SWAP_NEEDED(icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_ID),
			(uint16_t)SWAP_NEEDED(icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ)
		);
	}
	printf("\n");
}

void	print_time_exceeded_line(
	const t_ping* ping,
	const ip_header_t* ip_header,
	ip_header_t* original_ip,
	size_t icmp_whole_len,
	size_t original_icmp_whole_len
) {
	printf("%zu bytes from ", icmp_whole_len);
	print_address(ping, ip_header->IP_HEADER_SRC);
	printf(": Time to live exceeded\n");

	if (ping->prefs.verbose) {
		flip_endian_ip(original_ip);
		print_original_ip_dump(original_ip);
		print_original_ip_table(original_ip);
		flip_endian_ip(original_ip);

		if (original_icmp_whole_len >= 8) {
			const size_t			original_ip_header_len = original_ip->IP_HEADER_HL * 4;
			const icmp_header_t*	original_icmp_header = (icmp_header_t*)((void*)original_ip + original_ip_header_len);
			print_original_icmp_table(original_icmp_header, original_icmp_whole_len);
		}
	}
}
