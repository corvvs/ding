#include "ping.h"

extern int	g_is_little_endian;

#define SET_VR(current, new)	current = (new > current) ? new : current

// バリデーションの結果は4種類:
// - VR_ACCEPTED: エラー表示なし, 受信カウントを増やす
// - (VR_WARNING:  エラー表示あり, 受信カウントを増やす -> バリデーション時にエラーを出した後, VR_ACCEPTED に変更する)
// - VR_IGNORED:  エラー表示なし, 受信カウントを増やさない
// - (VR_REJECTED: エラー表示あり, 受信カウントを増やさない -> バリデーション時にエラーを出した後, VR_IGNORED に変更する)

// 生の受信データについてのバリデーション
t_validation_result	validate_receipt_raw_data(size_t recv_size) {
	if (recv_size < sizeof(ip_header_t)) {
		DEBUGERR("recv_size < sizeof(ip_header_t): %zu < %zu", recv_size, sizeof(ip_header_t));
		return VR_IGNORED;
	}
	return VR_ACCEPTED;
}

// IPレベルのバリデーション
t_validation_result	validate_receipt_ip(
	size_t recv_size,
	const ip_header_t* receipt_ip_header,
	const socket_address_in_t* addr_to
) {
	// TODO: 受信サイズ == トータルサイズ であることを確認する
	if (recv_size != receipt_ip_header->tot_len) {
		DEBUGERR("size doesn't match: rv: %zu, tot_len: %u", recv_size, receipt_ip_header->tot_len);
		return VR_IGNORED;
	}
	const size_t	ip_header_len = receipt_ip_header->ihl * 4;
	DEBUGOUT("ip_header_len: %zu", ip_header_len);
	// CHECK: ip_header_len >= sizeof(ip_header_t)
	if (recv_size < ip_header_len) {
		DEBUGERR("recv_size < ip_header_len: %zu < %zu", recv_size, ip_header_len);
		return VR_IGNORED;
	}

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
		DEBUGWARN("address_request_to != address_reply_from: %u != %u", address_request_to, address_reply_from);
	}
	// CHECK: IPヘッダ以降の部分がICMPヘッダを保持するのに十分なサイズを持つ
	const size_t		icmp_whole_len = recv_size  - ip_header_len;
	DEBUGOUT("icmp_whole_len: %zu", icmp_whole_len);
	if (icmp_whole_len < sizeof(icmp_header_t)) {
		DEBUGERR("icmp_whole_len < sizeof(icmp_header_t): %zu < %zu", icmp_whole_len, sizeof(icmp_header_t));
		return VR_IGNORED;
	}
	return VR_ACCEPTED;
}

// ICMPレベルのバリデーション
// 受信したものが「先立って自分が送ったICMP Echo Request」に対応する ICMP Echo Reply であることを確認する.
// 具体的には:
//   - 
// - IP
//   - [VR_WARNING] Reply の送信元が, Request の送信先であること
// - ICMP
//   - [VR_IGNORED] Type = 0 であること -> 違ったらスルー
//   - [VR_REJECTED] 長さが適切である(=データの先頭に timeval_t が1つ入る余裕がある)こと
//   - [VR_IGNORED] IDが一致すること
//   - [VR_WARNING] チェックサムがgoodであること
t_validation_result	validate_receipt_icmp(
	const t_ping* ping,
	void* receipt_icmp,
	size_t icmp_whole_len
) {
	t_validation_result	result = VR_ACCEPTED;

	icmp_header_t*	receipt_icmp_header = (icmp_header_t*)receipt_icmp;

	// CHECK: is ID correct?
	uint16_t	receipt_id = receipt_icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_ID;
	uint16_t	my_id = SWAP_NEEDED(ping->icmp_header_id);
	if (receipt_id != my_id) {
		DEBUGERR("receipt_id != my_id: %u != %u", receipt_id, my_id);
		return VR_IGNORED;
	}

	// CHECK: Type == 0?
	if (receipt_icmp_header->ICMP_HEADER_TYPE != 0) {
		DEBUGOUT("receipt_icmp_header->ICMP_HEADER_TYPE != 0: %u != 0", receipt_icmp_header->ICMP_HEADER_TYPE);
		return VR_IGNORED;
	}

	// CHECK: checksum is good?
	uint16_t	receipt_checksum = receipt_icmp_header->ICMP_HEADER_CHECKSUM;
	receipt_icmp_header->ICMP_HEADER_CHECKSUM = 0;
	DEBUGOUT("icmp_whole_len: %zu", icmp_whole_len);
	uint16_t	derived_checksum = derive_icmp_checksum(receipt_icmp, icmp_whole_len);
	DEBUGOUT("checksum: receipt: %u derived: %u", receipt_checksum, derived_checksum);
	if (receipt_checksum != derived_checksum) {
		DEBUGWARN("%s", "checksum is bad");
		SET_VR(result, VR_ACCEPTED);
	}

	// CHECK: is icmp_whole_len long enough?
	if (icmp_whole_len < sizeof(timeval_t)) {
		DEBUGERR("icmp_whole_len < sizeof(timeval_t): %zu < %zu", icmp_whole_len, sizeof(timeval_t));
		SET_VR(result, VR_IGNORED);
	}

	return result;
}