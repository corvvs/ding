#include "ping.h"

extern int	g_is_little_endian;

// 受信したものが「先立って自分が送ったICMP Echo Request」に対応する ICMP Echo Reply であることを確認する.
// 具体的には:
// - IP
//   - Reply の送信元が, Request の送信先であること
// - ICMP
//   - Type = 0 であること
//   - チェックサムがgoodであること
int	validate_receipt_data(
	const socket_address_in_t* addr_to,
	const ip_header_t* receipt_ip_header,
	void* receipt_icmp,
	size_t icmp_len
) {
	// CHECK: IP: Reply の送信元 == Request の送信先
	const uint32_t	address_request_to = SWAP_NEEDED(addr_to->sin_addr.s_addr);
	const uint32_t	address_reply_from = receipt_ip_header->saddr;
	DEBUGOUT("address_request_to: %u.%u.%u.%u",
		(address_request_to >> 24) & 0xff,
		(address_request_to >> 16) & 0xff,
		(address_request_to >> 8) & 0xff,
		(address_request_to >> 0) & 0xff
	);
	DEBUGOUT("address_reply_from: %u.%u.%u.%u",
		(address_reply_from >> 24) & 0xff,
		(address_reply_from >> 16) & 0xff,
		(address_reply_from >> 8) & 0xff,
		(address_reply_from >> 0) & 0xff
	);
	if (address_request_to != address_reply_from) {
		DEBUGERR("address_request_to != address_reply_from: %u != %u", address_request_to, address_reply_from);
		return -1;
	}

	icmp_header_t*	receipt_icmp_header = (icmp_header_t*)receipt_icmp;
	// CHECK: ICMP: Type = 0
	if (receipt_icmp_header->ICMP_HEADER_TYPE != 0) {
		DEBUGERR("receipt_icmp_header->ICMP_HEADER_TYPE != 0: %u != 0", receipt_icmp_header->ICMP_HEADER_TYPE);
		return -1;
	}

	// CHECK: checksum is good?
	uint16_t	receipt_checksum = receipt_icmp_header->ICMP_HEADER_CHECKSUM;
	receipt_icmp_header->ICMP_HEADER_CHECKSUM = 0;
	DEBUGOUT("icmp_len: %zu", icmp_len);
	uint16_t	derived_checksum = derive_icmp_checksum(receipt_icmp, icmp_len);
	DEBUGOUT("checksum: receipt: %u derived: %u", receipt_checksum, derived_checksum);
	if (receipt_checksum != derived_checksum) {
		DEBUGERR("%s", "checksum is bad");
		return -1;
	}

	return 0;
}