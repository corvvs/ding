#include "ping.h"
extern int	g_is_little_endian;

// NOTE: DO NOTHING
void	print_unexpected_icmp(const t_ping* ping, t_acceptance* acceptance) {
	(void)ping;
	(void)acceptance;
}

// IPヘッダをダンプする
static void	print_embedded_ip_dump(const ip_header_t* ip_header) {
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
static void	print_embedded_ip_table(const ip_header_t* ip_header) {
	// NOTE: どうやら列ごとに右詰めらしい

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
static void	print_embedded_icmp_table(const icmp_header_t* icmp_header, size_t embedded_icmp_whole_len) {
	printf("ICMP: type %u, code %u, size %zu",
		icmp_header->ICMP_HEADER_TYPE,
		icmp_header->ICMP_HEADER_CODE,
		embedded_icmp_whole_len
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

void	print_time_exceeded(
	const t_ping* ping,
	const t_acceptance* acceptance
) {
	const ip_header_t*		ip_header = acceptance->ip_header;
	icmp_header_t*			icmp_header = acceptance->icmp_header;
	const size_t			icmp_whole_len = acceptance->icmp_whole_len;
	icmp_detailed_header_t*	dicmp = (icmp_detailed_header_t*)icmp_header;
	ip_header_t*			embedded_ip = (ip_header_t*)&(dicmp->ICMP_DHEADER_ORIGINAL_IP);
	const size_t			embedded_ip_whole_len = icmp_whole_len - ((void*)embedded_ip - (void*)dicmp);
	const size_t			embedded_ip_header_len = embedded_ip->IP_HEADER_HL * 4;
	const size_t			embedded_icmp_whole_len = embedded_ip_whole_len - embedded_ip_header_len;
	// NOTE: ペイロードのオリジナルICMPの中身はカットされる可能性がある
	// (RFCでは"64 bits of Data Datagram"としか書かれていない)
	// ので, チェックサムは検証しない
	if (!ping->received_ipheader_modified) {
		flip_endian_ip(embedded_ip);
	}
	printf("%zu bytes from ", icmp_whole_len);
	print_address_struct(ping, &ip_header->IP_HEADER_SRC);
	printf(": Time to live exceeded\n");

	if (ping->prefs.verbose) {
		flip_endian_ip(embedded_ip);
		print_embedded_ip_dump(embedded_ip);
		print_embedded_ip_table(embedded_ip);
		flip_endian_ip(embedded_ip);

		if (embedded_icmp_whole_len >= 8) {
			const size_t			embedded_ip_header_len = embedded_ip->IP_HEADER_HL * 4;
			const icmp_header_t*	embedded_icmp_header = (icmp_header_t*)((void*)embedded_ip + embedded_ip_header_len);
			print_embedded_icmp_table(embedded_icmp_header, embedded_icmp_whole_len);
		}
	}
}
